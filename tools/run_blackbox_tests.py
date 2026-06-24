#!/usr/bin/env python3
"""Blackbox TCP tests for MiniRedis17.

Runs a real miniredis-server process and validates core protocol behavior over
TCP. Uses only the Python standard library.
"""

import argparse
import os
import shutil
import signal
import socket
import subprocess
import sys
import tempfile
import time


def resp_encode(*args):
    out = [f"*{len(args)}\r\n".encode()]
    for arg in args:
        if isinstance(arg, bytes):
            data = arg
        else:
            data = str(arg).encode()
        out.append(f"${len(data)}\r\n".encode())
        out.append(data)
        out.append(b"\r\n")
    return b"".join(out)


class RespReader:
    def __init__(self, sock, timeout=3.0):
        self.sock = sock
        self.sock.settimeout(timeout)
        self.buf = b""

    def _read_more(self):
        chunk = self.sock.recv(65536)
        if not chunk:
            raise EOFError("connection closed")
        self.buf += chunk

    def _line(self):
        while b"\r\n" not in self.buf:
            self._read_more()
        idx = self.buf.index(b"\r\n")
        line = self.buf[:idx]
        self.buf = self.buf[idx + 2:]
        return line

    def read(self):
        while not self.buf:
            self._read_more()
        prefix = self.buf[:1]
        self.buf = self.buf[1:]
        if prefix == b"+":
            return self._line().decode()
        if prefix == b"-":
            return "ERR:" + self._line().decode()
        if prefix == b":":
            return int(self._line())
        if prefix == b"$":
            n = int(self._line())
            if n == -1:
                return None
            while len(self.buf) < n + 2:
                self._read_more()
            data = self.buf[:n]
            tail = self.buf[n:n + 2]
            if tail != b"\r\n":
                raise AssertionError("bad bulk string terminator")
            self.buf = self.buf[n + 2:]
            return data.decode(errors="replace")
        if prefix == b"*":
            n = int(self._line())
            if n == -1:
                return None
            return [self.read() for _ in range(n)]
        raise AssertionError(f"unknown RESP prefix: {prefix!r}")


def free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def wait_for_port(port, timeout=5.0):
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            with socket.create_connection(("127.0.0.1", port), timeout=0.2):
                return True
        except OSError:
            time.sleep(0.05)
    return False


def connect(port):
    return socket.create_connection(("127.0.0.1", port), timeout=3.0)


def write_config(path, port, appendonly=False, aof_path=None):
    with open(path, "w") as f:
        f.write("bind 127.0.0.1\n")
        f.write(f"port {port}\n")
        f.write(f"appendonly {'yes' if appendonly else 'no'}\n")
        if aof_path:
            f.write(f"appendfilename {aof_path}\n")
        f.write("appendfsync always\n")
        f.write("active-expire-interval-ms 50\n")


def start_server(server, config):
    proc = subprocess.Popen([server, "--config", config], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return proc


def stop_server(proc):
    if proc.poll() is not None:
        return
    proc.send_signal(signal.SIGTERM)
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait(timeout=5)
        raise AssertionError("server did not exit after SIGTERM")


def test_ping(port):
    with connect(port) as s:
        rd = RespReader(s)
        s.sendall(resp_encode("PING"))
        assert rd.read() == "PONG"


def test_set_get(port):
    with connect(port) as s:
        rd = RespReader(s)
        s.sendall(resp_encode("SET", "k", "v"))
        assert rd.read() == "OK"
        s.sendall(resp_encode("GET", "k"))
        assert rd.read() == "v"


def test_pipeline(port):
    with connect(port) as s:
        rd = RespReader(s)
        s.sendall(resp_encode("SET", "a", "1") + resp_encode("INCR", "a") + resp_encode("GET", "a"))
        assert rd.read() == "OK"
        assert rd.read() == 2
        assert rd.read() == "2"


def test_split_command(port):
    with connect(port) as s:
        rd = RespReader(s)
        data = resp_encode("SET", "split", "yes")
        for b in data:
            s.sendall(bytes([b]))
        assert rd.read() == "OK"


def test_half_close(port):
    with connect(port) as s:
        rd = RespReader(s)
        s.sendall(resp_encode("SET", "half", "close"))
        s.shutdown(socket.SHUT_WR)
        assert rd.read() == "OK"


def test_ttl(port):
    with connect(port) as s:
        rd = RespReader(s)
        s.sendall(resp_encode("SET", "ttl-key", "x", "PX", "200"))
        assert rd.read() == "OK"
        time.sleep(0.35)
        s.sendall(resp_encode("GET", "ttl-key"))
        assert rd.read() is None


def test_quit(port):
    with connect(port) as s:
        rd = RespReader(s)
        s.sendall(resp_encode("QUIT"))
        assert rd.read() == "OK"
        s.settimeout(1.0)
        assert s.recv(1) == b""


def test_aof_recovery(server, tmpdir):
    port = free_port()
    config = os.path.join(tmpdir, "aof.conf")
    aof = os.path.join(tmpdir, "appendonly.aof")
    write_config(config, port, True, aof)
    proc = start_server(server, config)
    try:
        if not wait_for_port(port):
            raise AssertionError("AOF server did not start")
        with connect(port) as s:
            rd = RespReader(s)
            s.sendall(resp_encode("SET", "persist", "ok"))
            assert rd.read() == "OK"
    finally:
        stop_server(proc)

    proc = start_server(server, config)
    try:
        if not wait_for_port(port):
            raise AssertionError("AOF server did not restart")
        with connect(port) as s:
            rd = RespReader(s)
            s.sendall(resp_encode("GET", "persist"))
            assert rd.read() == "ok"
    finally:
        stop_server(proc)


def run_shared_tests(server, tmpdir):
    port = free_port()
    config = os.path.join(tmpdir, "server.conf")
    write_config(config, port, False)
    proc = start_server(server, config)
    try:
        if not wait_for_port(port):
            out, err = proc.communicate(timeout=1) if proc.poll() is not None else (b"", b"")
            raise AssertionError("server did not start: " + err.decode(errors="replace"))
        for name, fn in [
            ("PING", test_ping),
            ("SET/GET", test_set_get),
            ("pipeline", test_pipeline),
            ("split command", test_split_command),
            ("half close", test_half_close),
            ("TTL", test_ttl),
            ("QUIT", test_quit),
        ]:
            print(f"  {name} ...", end=" ", flush=True)
            fn(port)
            print("OK")
    finally:
        stop_server(proc)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--server", default=None)
    args = parser.parse_args()
    server = args.server
    if server is None:
        for d in ("build-debug", "build", "build-release"):
            candidate = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", d, "miniredis-server"))
            if os.path.exists(candidate):
                server = candidate
                break
    if not server or not os.path.exists(server):
        print("miniredis-server not found", file=sys.stderr)
        return 1
    server = os.path.abspath(server)
    tmpdir = tempfile.mkdtemp(prefix="miniredis_blackbox_")
    try:
        print(f"Using server: {server}")
        run_shared_tests(server, tmpdir)
        print("  AOF recovery ...", end=" ", flush=True)
        test_aof_recovery(server, tmpdir)
        print("OK")
        print("All blackbox tests passed")
        return 0
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
