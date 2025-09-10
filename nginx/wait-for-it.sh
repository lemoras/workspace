#!/bin/sh
# wait-for-it.sh
# Usage: ./wait-for-it.sh host:port [-t timeout] -- command args
# Basit versiyon
hostport="$1"
shift
cmd="$@"

host=$(echo $hostport | cut -d: -f1)
port=$(echo $hostport | cut -d: -f2)

echo "⏳ Waiting for $host:$port..."
while ! nc -z "$host" "$port"; do
  sleep 0.5
done

echo "✅ $host:$port is up, starting command..."
exec $cmd
