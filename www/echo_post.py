#!/usr/bin/env python3

import sys
import os

# Ler corpo da requisição
content_length = int(os.environ.get('CONTENT_LENGTH', 0))
post_data = sys.stdin.read(content_length) if content_length > 0 else ""

# Print headers
print("Content-Type: text/html; charset=utf-8")
print()

# Print HTML
print("<!DOCTYPE html>")
print("<html>")
print("<head><title>POST Echo</title></head>")
print("<body>")
print("    <h1>POST Data Received</h1>")
print(f"    <p><strong>Content-Length:</strong> {content_length}</p>")
print(f"    <p><strong>Content-Type:</strong> {os.environ.get('CONTENT_TYPE', 'N/A')}</p>")
print("    <h2>Body:</h2>")
print(f"    <pre>{post_data}</pre>")
print("</body>")
print("</html>")