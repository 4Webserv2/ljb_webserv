#!/usr/bin/env python3

import sys
import os

# Print headers
print("Content-Type: text/html; charset=utf-8")
print("X-Powered-By: Python/3")
print()  # Empty line separates headers from body

# Print HTML
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <meta charset='UTF-8'>")
print("    <title>CGI Environment Test</title>")
print("    <style>")
print("        body { font-family: monospace; padding: 20px; }")
print("        table { border-collapse: collapse; width: 100%; }")
print("        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }")
print("        th { background-color: #4CAF50; color: white; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <h1>🚀 CGI Environment Variables</h1>")
print("    <table>")
print("        <tr><th>Variable</th><th>Value</th></tr>")

# Listar todas as variáveis de ambiente
for key in sorted(os.environ.keys()):
    value = os.environ[key]
    # Escapar HTML
    value = value.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')
    print(f"        <tr><td><strong>{key}</strong></td><td>{value}</td></tr>")

print("    </table>")
print("</body>")
print("</html>")