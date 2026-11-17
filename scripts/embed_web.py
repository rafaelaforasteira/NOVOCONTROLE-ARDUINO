from pathlib import Path

# Ferramenta usada no computador, não no ESP32 nem no navegador.
# Copia os arquivos web para textos C++ para o ESP32 servir a página pela rede.
# Execute novamente após alterar HTML, CSS ou JS e regrave a placa.
root = Path(__file__).resolve().parents[1]
# Nome do arquivo original -> nome da constante que o firmware usa.
assets = {
    "index.html": "INDEX_HTML",
    "styles.css": "STYLES_CSS",
    "app.js": "APP_JS",
}
header = "#pragma once\n#include <Arduino.h>\n\n// Gerado a partir dos arquivos da raiz. Execute python3 scripts/embed_web.py.\n"
for filename, name in assets.items():
    content = (root / filename).read_text(encoding="utf-8")
    if filename == "index.html":
        marcador = '<body data-modo="demo">'
        if content.count(marcador) != 1:
            raise ValueError("Marcador do modo demonstração ausente ou duplicado")
        content = content.replace(marcador, '<body data-modo="esp32">')
    # Protege o delimitador da string C++ para não gerar um header inválido.
    if ')CARRINHO"' in content:
        raise ValueError("Delimitador reservado em " + filename)
    # PROGMEM mantém os textos na flash; a raw string preserva aspas e linhas.
    header += f'\nconst char {name}[] PROGMEM = R"CARRINHO({content})CARRINHO";\n'
(root / "firmware/carrinho/web_assets.h").write_text(header, encoding="utf-8")
print("web_assets.h atualizado")
