# Controle simples

Abra index.html. Não precisa instalar bibliotecas nem compilar: HTML monta a página, CSS cuida da aparência e JavaScript lê os botões e envia os comandos.

Segure Frente, Ré, Esquerda ou Direita para mover; solte para parar. Também funciona com as setas do teclado e com Espaço/Enter no botão focado. Escape também para. No celular, gire o aparelho; Tela cheia tenta fixar na horizontal quando o navegador permite.

## Conectar depois ao ESP32

1. Em app.js, altere `DEMONSTRACAO = true` para `false`.
2. Sirva index.html, styles.css e app.js pelo servidor HTTP do carrinho. O endereço vazio usa o próprio servidor. Para outro destino, ajuste ENDERECO e configure CORS no firmware; HTTPS pode bloquear acesso a HTTP.
3. O protocolo é o mesmo do projeto anterior: `GET /comando?acao=frente`, `re`, `esquerda`, `direita` ou `parar`.

A demonstração não faz requisições para o carrinho. O firmware não foi alterado. Uma perda de rede pode impedir a entrega da parada: acrescente ao firmware uma parada automática por tempo sem comandos antes do uso físico e adapte a página para repetir comandos nesse intervalo, se necessário.

Esta pasta é independente da página original com joystick.

