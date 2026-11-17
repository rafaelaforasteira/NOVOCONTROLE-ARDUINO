#pragma once
#include <Arduino.h>

// Gerado a partir dos arquivos da raiz. Execute python3 scripts/embed_web.py.

const char INDEX_HTML[] PROGMEM = R"CARRINHO(<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
  <meta name="theme-color" content="#fbfcf8">
  <title>Controle do carrinho</title>
  <link rel="stylesheet" href="styles.css">
  <script src="app.js" defer></script>
</head>
<body data-modo="esp32">
  <main class="controle" aria-label="Controle do carrinho">
    <div class="direcoes" aria-label="Controle de direção">
      <button class="direcao frente" data-comando="frente" type="button"><span class="seta">↑</span><span>Frente</span></button>
      <button class="direcao esquerda" data-comando="esquerda" type="button"><span class="seta">←</span><span>Esquerda</span></button>
      <button class="direcao direita" data-comando="direita" type="button"><span class="seta">→</span><span>Direita</span></button>
      <button class="direcao re" data-comando="re" type="button"><span class="seta">↓</span><span>Ré</span></button>
    </div>
    <div class="informacoes">
      <div class="movimento"><span class="indicador"></span><div><span class="legenda">COMANDO ATUAL</span><output id="movimento" aria-live="polite">Parado</output></div></div>
      <p class="instrucao">Segure para mover. Solte para parar.</p>
      <p id="conexao" role="status">Modo demonstração</p>
    </div>
    <button id="tela" type="button" aria-label="Entrar em tela cheia" title="Tela cheia">⛶</button>
    <p id="aviso" role="status" hidden></p>
  </main>
</body>
</html>
)CARRINHO";

const char STYLES_CSS[] PROGMEM = R"CARRINHO(/* A própria página é o controle: sem cartão, borda ou margem externa. */
:root{font-family:Arial,Helvetica,sans-serif;color:#354b32;background:#fbfcf8;font-synthesis:none}
*{box-sizing:border-box}
body{margin:0}
.controle{min-height:100svh;width:100%;position:relative;display:flex;align-items:center;gap:clamp(40px,9vw,160px);padding: max(24px,env(safe-area-inset-top)) max(24px,env(safe-area-inset-right)) max(24px,env(safe-area-inset-bottom)) max(28px,env(safe-area-inset-left))}
.direcoes{--tamanho:clamp(72px,23svh,120px);display:grid;grid-template-columns:repeat(3,var(--tamanho));grid-template-rows:repeat(3,var(--tamanho));gap:12px;flex-shrink:0}
button{font:inherit;cursor:pointer;-webkit-tap-highlight-color:transparent}
button:focus-visible{outline:3px solid #2b7552;outline-offset:4px}
.direcao{border:1px solid #d4ddcb;border-bottom:4px solid #c9d3bc;background:#e9efdf;color:#354b32;border-radius:19px;display:flex;flex-direction:column;justify-content:center;align-items:center;gap:5px;touch-action:none;user-select:none;-webkit-user-select:none;-webkit-touch-callout:none}
.seta{font-size:38px;line-height:1.1}
.direcao>span:last-child{font-size:13px;font-weight:600}
@media(hover:hover){.direcao:hover{background:#dfe8d0}}
.direcao.ativo{background:#325b3d;color:white;border-color:#274930;transform:translateY(3px);border-bottom-width:1px}
.frente{grid-area:1/2}.esquerda{grid-area:2/1}.direita{grid-area:2/3}.re{grid-area:3/2}
.movimento{display:flex;align-items:center;gap:15px}
.indicador{width:9px;height:9px;border-radius:50%;background:#9baa91;box-shadow:0 0 0 6px #eff2e9}
.movendo .indicador{background:#3c784d}
.legenda{display:block;font-size:10px;letter-spacing:1.8px;color:#64735d;margin-bottom:8px}
output{display:block;font-size:24px;font-weight:600;min-width:110px}
#tela{position:absolute;right:max(16px,env(safe-area-inset-right));top:max(12px,env(safe-area-inset-top));border:0;background:transparent;color:#63725c;font-size:28px;width:44px;height:44px}
#aviso{position:absolute;bottom:10px;left:24px;right:24px;text-align:center;color:#64735d;font-size:12px}
@media(orientation:landscape) and (max-height:500px){.controle{gap:clamp(30px,10vw,100px)}.direcoes{--tamanho:clamp(52px,23svh,100px);gap:8px}.direcao{border-radius:15px}.seta{font-size:30px}.direcao>span:last-child{font-size:11px}}
@media(orientation:portrait){.controle{flex-direction:column;align-items:flex-start;justify-content:center;gap:40px;padding-left:max(20px,env(safe-area-inset-left))}.direcoes{--tamanho:clamp(64px,25vw,110px);gap:10px}.movimento{margin-left:10px}}

.informacoes{min-width:0}
.instrucao,#conexao{font-size:13px;line-height:1.35;margin:12px 0 0}
#conexao{font-weight:600}
@media(orientation:portrait){.movimento{margin-left:0}}
)CARRINHO";

const char APP_JS[] PROGMEM = R"CARRINHO('use strict';

// O HTML embutido no firmware habilita o envio para a própria origem.
const DEMONSTRACAO = document.body.dataset.modo !== 'esp32';
const conexao = document.querySelector('#conexao');
if (!DEMONSTRACAO) conexao.textContent = 'Controle via ESP32';

const nomes = { frente: 'Frente', re: 'Ré', esquerda: 'Esquerda', direita: 'Direita', parar: 'Parado' };
const botoes = document.querySelectorAll('[data-comando]');

let atual = 'parar';
let ponteiro = null;
let tecla = null;
let enviando = false;
let proximo = null;
let paradaPendente = false;

// Serializa pedidos e preserva a parada mesmo se outro botão for acionado.
async function enviar(comando) {
  if (DEMONSTRACAO) return;
  if (comando === 'parar') {
    paradaPendente = true;
    proximo = null;
  } else {
    proximo = comando;
  }
  if (enviando) return;
  enviando = true;
  while (paradaPendente || proximo !== null) {
    const acao = paradaPendente ? 'parar' : proximo;
    if (paradaPendente) paradaPendente = false;
    else proximo = null;

    const controle = new AbortController();
    const limite = setTimeout(() => controle.abort(), 400);
    try {
      const resposta = await fetch('/comando?acao=' + acao, {
        cache: 'no-store', signal: controle.signal, keepalive: true
      });
      if (!resposta.ok) throw new Error('Comando recusado');
      conexao.textContent = 'ESP32 respondeu';
    } catch {
      conexao.textContent = acao === 'parar' ? 'Parada não confirmada' : 'Falha de comunicação';
      if (acao !== 'parar') parar();
    } finally {
      clearTimeout(limite);
    }
  }
  enviando = false;
}

if (!DEMONSTRACAO) {
  // O ESP32 desliga os motores se estas renovações deixarem de chegar.
  setInterval(() => {
    if (!enviando && atual !== 'parar') enviar(atual);
  }, 150);
}

function mover(comando) {
  if (atual === comando) return;
  atual = comando;
  document.querySelector('#movimento').textContent = nomes[comando];
  document.body.classList.toggle('movendo', comando !== 'parar');
  botoes.forEach(botao => {
    const ativo = botao.dataset.comando === comando;
    botao.classList.toggle('ativo', ativo);
    botao.setAttribute('aria-pressed', String(ativo));
  });
  enviar(comando);
}

function parar() {
  ponteiro = null;
  tecla = null;
  mover('parar');
}

// O mesmo evento atende tanto ao dedo quanto ao mouse.
botoes.forEach(botao => {
  botao.setAttribute('aria-pressed', 'false');
  botao.addEventListener('pointerdown', evento => {
    if (ponteiro !== null || tecla !== null || evento.button !== 0) return;
    evento.preventDefault();
    ponteiro = evento.pointerId;
    botao.setPointerCapture(ponteiro);
    mover(botao.dataset.comando);
  });
  ['pointerup', 'pointercancel', 'lostpointercapture'].forEach(tipo => {
    botao.addEventListener(tipo, evento => {
      if (evento.pointerId === ponteiro) parar();
    });
  });
  // Espaço/Enter também funcionam com o botão focado.
  botao.addEventListener('keydown', evento => {
    if (![' ', 'Enter'].includes(evento.key) || evento.repeat || ponteiro !== null || tecla !== null) return;
    evento.preventDefault();
    tecla = evento.key;
    mover(botao.dataset.comando);
  });
  botao.addEventListener('blur', parar);
});

const setas = { ArrowUp: 'frente', ArrowDown: 're', ArrowLeft: 'esquerda', ArrowRight: 'direita' };
document.addEventListener('keydown', evento => {
  if (evento.key === 'Escape') parar();
  if (!setas[evento.key]) return;
  evento.preventDefault();
  if (ponteiro !== null || tecla !== null || evento.repeat) return;
  tecla = evento.key;
  mover(setas[evento.key]);
});
document.addEventListener('keyup', evento => {
  if (evento.key === tecla) { evento.preventDefault(); parar(); }
});

window.addEventListener('blur', parar);
window.addEventListener('pagehide', parar);
window.addEventListener('resize', parar);
document.addEventListener('visibilitychange', () => { if (document.hidden) parar(); });

// Alguns celulares permitem travar na horizontal depois de entrar em tela cheia.
document.querySelector('#tela').addEventListener('click', async () => {
  parar();
  try {
    if (document.fullscreenElement) await document.exitFullscreen();
    else {
      await document.documentElement.requestFullscreen();
      try { await screen.orientation.lock('landscape'); } catch { /* Gire o aparelho manualmente. */ }
    }
  } catch {
    const aviso = document.querySelector('#aviso');
    aviso.hidden = false;
    aviso.textContent = 'Tela cheia indisponível neste navegador. Gire o celular para usar na horizontal.';
  }
});
document.addEventListener('fullscreenchange', () => {
  parar();
  document.querySelector('#tela').setAttribute('aria-label', document.fullscreenElement ? 'Sair da tela cheia' : 'Entrar em tela cheia');
});
)CARRINHO";
