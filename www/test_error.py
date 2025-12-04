#!/usr/bin/env python3

import sys

# Simular erro com status customizado
print("Status: 500 Internal Server Error")
print("Content-Type: text/html")
print()

print("<html><body>")
print("<h1>Erro simulado no CGI</h1>")
print("<p>Este é um teste de erro controlado.</p>")
print("</body></html>")

# Sair com código de erro
sys.exit(1)