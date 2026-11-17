'use strict';

// Na prévia, os botões só mudam a interface. Para usar no carrinho,
// mude DEMONSTRACAO para false e sirva estes arquivos pelo ESP32.
const DEMONSTRACAO = true;
const ENDERECO = ''; // Vazio = mesmo servidor da página. Ex.: 'http://192.168.10.108'
const nomes = { frente: 'Frente', re: 'Ré', esquerda: 'Esquerda', direita: 'Direita', parar: 'Parado' };
const botoes = document.querySelectorAll('[data-comando]');

let atual = 'parar';
let ponteiro = null;
let tecla = null;
let enviando = false;
let proximo = null;

// Uma requisição por vez evita que comandos antigos ultrapassem a parada.
async function enviar(comando) {
  if (DEMONSTRACAO) return;
  proximo = comando;
  if (enviando) return;
  enviando = true;
  while (proximo !== null) {
    const acao = proximo;
    proximo = null;
    try {
      const resposta = await fetch(ENDERECO + '/comando?acao=' + acao, {
        cache: 'no-store', signal: AbortSignal.timeout(1000), keepalive: true
      });
      if (!resposta.ok) throw new Error('Sem resposta');


    } catch {
      console.warn('Sem conexão com o carrinho.');

      if (acao !== 'parar') parar();
    }
  }
  enviando = false;
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


