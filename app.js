'use strict';

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
