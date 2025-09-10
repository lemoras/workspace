#!/bin/sh
set -e

# Backend'leri bekle
./wait-for-it.sh lemoras-gateway:80
./wait-for-it.sh lemoras-ui:80

# Sertifika yoksa oluştur
if [ ! -f /etc/nginx/certs/dev.key ]; then
  mkdir -p /etc/nginx/certs
  openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
    -keyout /etc/nginx/certs/dev.key \
    -out /etc/nginx/certs/dev.crt \
    -subj "/C=TR/ST=Istanbul/L=Istanbul/O=Dev/OU=IT/CN=dev.local"
fi

echo "🚀 Starting nginx..."
exec nginx -g "daemon off;"
