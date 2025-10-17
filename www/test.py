#!/usr/bin/env python3

import sys
import os

# Print HTTP headers
print("Content-Type: text/html")
print("X-Custom-Header: CGI Test")
print()  # Empty line to separate headers from body

# Print HTML body
print("<html><body>")
print("<h1>CGI Test Successful</h1>")
print("<h2>Environment Variables:</h2>")
print("<ul>")
for key, value in sorted(os.environ.items()):
    print(f"<li><strong>{key}</strong>: {value}</li>")
print("</ul>")
print("</body></html>")