#!/usr/bin/env python3
"""
Simple RESP2 client for testing MiniRedis17.

Usage:
    python3 tools/resp_client.py [host] [port]
    python3 tools/resp_client.py 127.0.0.1 6380

Then type commands like:
    PING
    SET key value
    GET key
    LPUSH list a b c
    LRANGE list 0 -1
"""

import socket
import sys


def encode_command(args):
    """Encode a command as RESP Array of Bulk Strings."""
    result = f"*{len(args)}\r\n"
    for arg in args:
        result += f"${len(arg)}\r\n{arg}\r\n"
    return result.encode()


def read_response(sock):
    """Read a single RESP response from socket."""
    # Read type prefix
    prefix = b''
    while True:
        ch = sock.recv(1)
        if not ch:
            return None
        prefix += ch
        if ch == b'\n':
            break

    line = prefix.decode().strip()
    type_byte = line[0]

    if type_byte == '+':
        return line[1:]
    elif type_byte == '-':
        return f"(error) {line[1:]}"
    elif type_byte == ':':
        return int(line[1:])
    elif type_byte == '$':
        length = int(line[1:])
        if length == -1:
            return None
        data = b''
        while len(data) < length + 2:
            chunk = sock.recv(length + 2 - len(data))
            if not chunk:
                return None
            data += chunk
        return data[:-2].decode(errors='replace')
    elif type_byte == '*':
        count = int(line[1:])
        if count == -1:
            return None
        result = []
        for _ in range(count):
            result.append(read_response(sock))
        return result
    else:
        return f"(unknown) {line}"


def format_response(resp, indent=0):
    """Format a response for display."""
    prefix = " " * indent
    if resp is None:
        return f"{prefix}(nil)"
    elif isinstance(resp, list):
        if not resp:
            return f"{prefix}(empty array)"
        lines = []
        for i, item in enumerate(resp):
            if item is None:
                lines.append(f"{prefix}{i+1}) (nil)")
            elif isinstance(item, list):
                lines.append(f"{prefix}{i+1}) {format_response(item, indent+2)}")
            else:
                lines.append(f"{prefix}{i+1}) {item}")
        return '\n'.join(lines)
    else:
        return f"{prefix}{resp}"


def main():
    host = sys.argv[1] if len(sys.argv) > 1 else '127.0.0.1'
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 6380

    print(f"Connecting to {host}:{port}...")
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((host, port))
    except ConnectionRefusedError:
        print(f"Error: Connection refused. Is the server running on {host}:{port}?")
        sys.exit(1)

    print(f"Connected. Type commands (Ctrl+D to quit):")

    try:
        while True:
            try:
                line = input(f"{host}:{port}> ")
            except EOFError:
                print()
                break

            if not line.strip():
                continue

            args = line.strip().split()
            if not args:
                continue

            sock.sendall(encode_command(args))
            resp = read_response(sock)
            print(format_response(resp))
    finally:
        sock.close()


if __name__ == '__main__':
    main()
