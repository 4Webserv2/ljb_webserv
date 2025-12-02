/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: btaveira <btaveira@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 20:39:32 by jbergfel          #+#    #+#             */
/*   Updated: 2025/12/02 12:40:06 by btaveira         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpRequest.hpp"
# include "../includes/StringUtils.hpp"

HttpRequest::HttpRequest(){}

HttpRequest::~HttpRequest(){}

void HttpRequest::parseRequestLine(std::istringstream &stream, HttpParse &parse) {
    std::string line;
    if (std::getline(stream, line)) {
        // Remove \r se existir
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        
        std::istringstream first_line(line);
        first_line >> parse.method >> parse.uri >> parse.version;

        if (parse.method.empty() || parse.uri.empty() || parse.version.empty())
            throw std::runtime_error("Request line malformada: campos ausentes");
    }
}

std::string HttpRequest::unfoldHeader(const std::string &value) {
    // RFC 7230: Folded headers são obsoletos mas devem ser suportados
    // Um header "folded" tem \r\n seguido de espaço/tab
    std::string result = value;
    size_t pos = 0;
    
    while ((pos = result.find("\r\n", pos)) != std::string::npos) {
        // Verificar se após \r\n há um espaço ou tab
        if (pos + 2 < result.length() && 
            (result[pos + 2] == ' ' || result[pos + 2] == '\t')) {
            // Substituir \r\n + espaço por um único espaço
            result.replace(pos, 3, " ");
        } else {
            break;
        }
    }
    
    return result;
}

void HttpRequest::parseHeaders(std::istringstream &stream, HttpParse &parse) {
    std::string line;
    std::string currentHeader;
    std::string currentValue;
    
    while (std::getline(stream, line)) {
        // Remover \r se existir
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        
        // Fim dos headers (linha vazia)
        if (line.empty())
            break;
        
        // Verificar se é uma continuação (folded header)
        // Linha começa com espaço ou tab
        if (!line.empty() && (line[0] == ' ' || line[0] == '\t')) {
            // Continuação do header anterior
            if (!currentHeader.empty()) {
                currentValue += " " + StringUtils::trim(line);
            }
            continue;
        }
        
        // Se temos um header anterior, salvá-lo
        if (!currentHeader.empty()) {
            std::string normalizedKey = StringUtils::normalizeHeaderName(currentHeader);
            std::string trimmedValue = StringUtils::trim(currentValue);
            
            // Tratar headers duplicados
            if (parse.headers.count(normalizedKey) > 0) {
                // RFC 7230: Combinar valores com vírgula
                // Exceções: Set-Cookie, etc (não aplicável em requests)
                parse.headers[normalizedKey] += ", " + trimmedValue;
            } else {
                parse.headers[normalizedKey] = trimmedValue;
            }
        }
        
        // Processar novo header
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            currentHeader = line.substr(0, colonPos);
            currentValue = line.substr(colonPos + 1);
            
            // Unfold se necessário
            currentValue = unfoldHeader(currentValue);
        } else {
            // Header malformado - ignorar
            std::cerr << "AVISO: Header malformado ignorado: " << line << std::endl;
            currentHeader.clear();
            currentValue.clear();
        }
    }
    
    // Salvar último header se existir
    if (!currentHeader.empty()) {
        std::string normalizedKey = StringUtils::normalizeHeaderName(currentHeader);
        std::string trimmedValue = StringUtils::trim(currentValue);
        
        if (parse.headers.count(normalizedKey) > 0) {
            parse.headers[normalizedKey] += ", " + trimmedValue;
        } else {
            parse.headers[normalizedKey] = trimmedValue;
        }
    }
}

void HttpRequest::parseBody(std::istringstream &stream, HttpParse &parse) {
    std::string line, body;
    while (std::getline(stream, line)) {
        body += line + "\n";
    }
    parse.body = body;
}

HttpParse HttpRequest::httpParse(const std::string &rawRequest) {
    HttpParse parse;
    std::istringstream stream(rawRequest);
    parseRequestLine(stream, parse);
    parseHeaders(stream, parse);
    parseBody(stream, parse);
    return parse;
}

void HttpRequest::setPar(HttpParse parse) {
    this->_par = parse;
}

std::string HttpRequest::getMethod() const {
    return this->_par.method;
}

std::string HttpRequest::getUri() const {
    return this->_par.uri;
}

std::string HttpRequest::getVersion() const {
    return this->_par.version;
}

std::map<std::string, std::string> HttpRequest::getHeaders() const {
    return this->_par.headers;
}

std::string HttpRequest::getBody() const {
    return this->_par.body;
}

// Getter case-insensitive para um header específico
std::string HttpRequest::getHeader(const std::string &name) const {
    std::string normalizedName = StringUtils::normalizeHeaderName(name);
    
    std::map<std::string, std::string>::const_iterator it = this->_par.headers.find(normalizedName);
    if (it != this->_par.headers.end())
        return it->second;
    
    return "";
}

bool HttpRequest::hasHeader(const std::string &name) const {
    std::string normalizedName = StringUtils::normalizeHeaderName(name);
    return this->_par.headers.count(normalizedName) > 0;
}
