# Controle do carrinho

O ESP32 serve a página com os quatro botões e recebe os comandos pelo Wi-Fi local.

## Gravar e abrir

1. Abra `firmware/carrinho/carrinho.ino` na Arduino IDE com o suporte ESP32 instalado. Mantenha `web_assets.h` na mesma pasta.
2. Preencha `ssid` e `senha` para a rede do carrinho na sua cópia local. Selecione o modelo da placa e a porta USB, então grave o sketch.
3. Abra o Monitor Serial a 9600 baud e copie o endereço HTTP exibido após a conexão.
4. Conecte o celular ou computador à mesma rede e abra esse endereço no navegador.
5. Com as rodas suspensas, pressione e solte cada botão. Confira frente, ré, esquerda, direita e parada. Faça o teste de perda de comunicação em bancada.

A página aberta diretamente do repositório ou por um servidor estático roda em demonstração. A página servida pelo ESP32 envia `GET /comando?acao=frente|re|esquerda|direita|parar` para a própria origem. Também é possível usar as setas do teclado. Use um único controle por vez.

Ao segurar um botão, a página tenta renovar o comando a cada 150 ms. Ao soltar, envia `parar`. O firmware verifica se passaram 750 ms desde o último comando válido de movimento e desliga as saídas da ponte H quando esse prazo expira ou quando o ESP32 perde o Wi-Fi. Essa verificação roda no loop do firmware. As rotinas de motor e farol são as da primeira versão.

Depois de editar HTML, CSS ou JavaScript, execute `python3 scripts/embed_web.py` e regrave a placa para atualizar a página embutida.
