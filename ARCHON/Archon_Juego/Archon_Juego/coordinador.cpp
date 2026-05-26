#include "Coordinador.h"
#include <cmath>
#include <algorithm>

static constexpr float TAM_CASILLA = 70.f;
static constexpr float OFFSET_X = 100.f;
static constexpr float OFFSET_Y = 50.f;

Coordinador::Coordinador()
    : estado(EstadoJuego::MENU), menu(1280, 720)
{
    fuenteCargada = fuente.loadFromFile("assets/arial.ttf");
}

void Coordinador::inicializar() {
    menu.cargarRecursos();
}

void Coordinador::gestionarEventos(sf::RenderWindow& window, sf::Event& event) {
    if (estado == EstadoJuego::MENU) {
        EstadoMenu res = menu.procesarEventos(window, event);
        if (res == EstadoMenu::JUGANDO) estado = EstadoJuego::TABLERO;
        if (res == EstadoMenu::SALIR)   estado = EstadoJuego::SALIR;
        return;
    }

    if (estado == EstadoJuego::TABLERO) {
        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left)
            procesarClickTablero(event.mouseButton.x, event.mouseButton.y);
        if (event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::Escape)
            estado = EstadoJuego::MENU;
        return;
    }

    if (estado == EstadoJuego::ARENA_COMBATE) {
        if (event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::Escape)
            estado = EstadoJuego::TABLERO;
    }
}

void Coordinador::procesarClickTablero(int px, int py) {
    int col = static_cast<int>((px - OFFSET_X) / TAM_CASILLA);
    int fil = static_cast<int>((py - OFFSET_Y) / TAM_CASILLA);

    if (col < 0 || col >= Tablero::TAM || fil < 0 || fil >= Tablero::TAM) {
        selFila = selCol = -1;
        return;
    }

    if (selFila == -1) {
        selFila = fil;
        selCol = col;
    }
    else {
        bool movido = tablero.moverPieza(selFila, selCol, fil, col);
        if (movido && tablero.hayCombatePendiente()) {
            Pieza* at = tablero.getAtacante();
            Pieza* df = tablero.getDefensor();
            piezaAzulCombate = (at && at->getEquipo() == Equipo::Azul) ? at : df;
            piezaRojaCombate = (at && at->getEquipo() == Equipo::Rojo) ? at : df;
            iniciarCombate();
            estado = EstadoJuego::ARENA_COMBATE;
        }
        selFila = selCol = -1;
    }
}

void Coordinador::iniciarCombate() {
    posAzul = sf::Vector2f(200.f, 360.f);
    posRoja = sf::Vector2f(1080.f, 360.f);
    vidaAzulCombate = piezaAzulCombate ? piezaAzulCombate->getVida() : 100;
    vidaRojaCombate = piezaRojaCombate ? piezaRojaCombate->getVida() : 100;
    velAzul = 150.f + (piezaAzulCombate ? piezaAzulCombate->getVelocidad() * 20.f : 0.f);
    velRoja = 150.f + (piezaRojaCombate ? piezaRojaCombate->getVelocidad() * 20.f : 0.f);
    recargaAzul = recargaRoja = 0.f;
    proyectiles.clear();
    relojCombate.restart();
}

void Coordinador::actualizar(sf::RenderWindow& window) {
    if (estado == EstadoJuego::MENU) {
        menu.actualizar(window);
        return;
    }

    if (estado == EstadoJuego::TABLERO) {
        tablero.mueve(0.016);

        ResultadoVictoria res = tablero.comprobarVictoria();
        if (res == ResultadoVictoria::GanaAzul) {
            ganadorTexto = "GANA EL EQUIPO AZUL";
            estado = EstadoJuego::FIN_PARTIDA;
        }
        else if (res == ResultadoVictoria::GanaRojo) {
            ganadorTexto = "GANA EL EQUIPO ROJO";
            estado = EstadoJuego::FIN_PARTIDA;
        }
        return;
    }

    if (estado == EstadoJuego::ARENA_COMBATE) {
        float dt = relojCombate.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        actualizarCombate(dt);
        return;
    }
}
void Coordinador::actualizarCombate(float dt) {
    if (!piezaAzulCombate || !piezaRojaCombate) return;

    float vA = velAzul * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) posAzul.y -= vA;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) posAzul.y += vA;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) posAzul.x -= vA;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) posAzul.x += vA;

    float vR = velRoja * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    posRoja.y -= vR;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  posRoja.y += vR;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  posRoja.x -= vR;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) posRoja.x += vR;

    auto clamp = [](sf::Vector2f& p) {
        p.x = std::max(50.f, std::min(p.x, 1230.f));
        p.y = std::max(50.f, std::min(p.y, 670.f));
        };
    clamp(posAzul); clamp(posRoja);

    recargaAzul -= dt; recargaRoja -= dt;

    float intA = std::max(0.2f, 1.5f - piezaAzulCombate->getVelocidadAtaque() * 0.13f);
    float intR = std::max(0.2f, 1.5f - piezaRojaCombate->getVelocidadAtaque() * 0.13f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && recargaAzul <= 0.f) {
        dispararAzul(); recargaAzul = intA;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return) && recargaRoja <= 0.f) {
        dispararRoja(); recargaRoja = intR;
    }

    for (auto& p : proyectiles) {
        if (!p.activo) continue;
        p.forma.move(p.vel);
        auto pos = p.forma.getPosition();
        if (pos.x < 0 || pos.x > 1280 || pos.y < 0 || pos.y > 720) { p.activo = false; continue; }

        sf::FloatRect pb = p.forma.getGlobalBounds();
        if (p.esAzul) {
            if (pb.intersects({ posRoja.x - 24, posRoja.y - 24, 48, 48 })) {
                vidaRojaCombate = std::max(0, vidaRojaCombate - p.danio);
                p.activo = false;
            }
        }
        else {
            if (pb.intersects({ posAzul.x - 24, posAzul.y - 24, 48, 48 })) {
                vidaAzulCombate = std::max(0, vidaAzulCombate - p.danio);
                p.activo = false;
            }
        }
    }
    proyectiles.erase(std::remove_if(proyectiles.begin(), proyectiles.end(),
        [](const Proyectil& p) { return !p.activo; }), proyectiles.end());

    if (vidaAzulCombate <= 0 || vidaRojaCombate <= 0) {
        Pieza* perdedor = (vidaAzulCombate <= 0) ? piezaAzulCombate : piezaRojaCombate;
        if (vidaAzulCombate > 0 && piezaAzulCombate)
            piezaAzulCombate->setVida(vidaAzulCombate);
        if (vidaRojaCombate > 0 && piezaRojaCombate)
            piezaRojaCombate->setVida(vidaRojaCombate);

        tablero.resolverCombate(perdedor);
        tablero.finalizarTurno();
        piezaAzulCombate = piezaRojaCombate = nullptr;
        proyectiles.clear();
        estado = EstadoJuego::TABLERO;
    }
}

void Coordinador::dispararAzul() {
    sf::Vector2f dir = posRoja - posAzul;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 1.f) return;
    dir /= len;
    Proyectil p;
    p.forma.setRadius(6.f); p.forma.setOrigin(6.f, 6.f);
    p.forma.setFillColor(sf::Color(100, 180, 255));
    p.forma.setPosition(posAzul);
    p.vel = dir * (300.f + piezaAzulCombate->getAlcanceAtaque() * 15.f) / 60.f;
    p.danio = piezaAzulCombate->getFuerzaAtaque();
    p.esAzul = true;
    proyectiles.push_back(p);
}

void Coordinador::dispararRoja() {
    sf::Vector2f dir = posAzul - posRoja;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 1.f) return;
    dir /= len;
    Proyectil p;
    p.forma.setRadius(6.f); p.forma.setOrigin(6.f, 6.f);
    p.forma.setFillColor(sf::Color(255, 100, 100));
    p.forma.setPosition(posRoja);
    p.vel = dir * (300.f + piezaRojaCombate->getAlcanceAtaque() * 15.f) / 60.f;
    p.danio = piezaRojaCombate->getFuerzaAtaque();
    p.esAzul = false;
    proyectiles.push_back(p);
}

void Coordinador::dibujar(sf::RenderWindow& window) {
    if (estado == EstadoJuego::MENU) { menu.dibujar(window); return; }

    if (estado == EstadoJuego::TABLERO) {
        tablero.dibuja();
        if (selFila != -1) {
            sf::RectangleShape sel({ TAM_CASILLA - 2.f, TAM_CASILLA - 2.f });
            sel.setPosition(OFFSET_X + selCol * TAM_CASILLA, OFFSET_Y + selFila * TAM_CASILLA);
            sel.setFillColor(sf::Color(100, 255, 100, 120));
            sel.setOutlineColor(sf::Color::White);
            sel.setOutlineThickness(3.f);
            window.draw(sel);
        }
        return;
    }

    if (estado == EstadoJuego::ARENA_COMBATE) { dibujarCombate(window); return; }

    if (estado == EstadoJuego::FIN_PARTIDA && fuenteCargada) {
        sf::Text t(ganadorTexto, fuente, 60);
        t.setFillColor(sf::Color::White);
        sf::FloatRect bounds = t.getLocalBounds();
        t.setPosition((1280.f - bounds.width) / 2.f, 300.f);
        window.draw(t);
    }
}

void Coordinador::dibujarCombate(sf::RenderWindow& window) {
    static sf::Texture texCombate;
    static sf::Sprite sprCombate;
    static bool combateCargado = false;
    if (!combateCargado) {
        texCombate.loadFromFile("assets/combate_bueno.png");
        sprCombate.setTexture(texCombate);
        sprCombate.setScale(
            1280.f / texCombate.getSize().x,
            720.f / texCombate.getSize().y
        );
        combateCargado = true;
    }
    window.draw(sprCombate);

    sf::CircleShape sA(24.f); sA.setOrigin(24.f, 24.f);
    sA.setFillColor(sf::Color(70, 130, 220)); sA.setOutlineColor(sf::Color::White); sA.setOutlineThickness(2.f);
    sA.setPosition(posAzul); window.draw(sA);

    sf::CircleShape sR(24.f); sR.setOrigin(24.f, 24.f);
    sR.setFillColor(sf::Color(220, 60, 60)); sR.setOutlineColor(sf::Color::White); sR.setOutlineThickness(2.f);
    sR.setPosition(posRoja); window.draw(sR);

    for (auto& p : proyectiles) if (p.activo) window.draw(p.forma);

    
    // Cargar marcos de barras de vida
    static sf::Texture texBarraAzul, texBarraRoja;
    static sf::Sprite sprBarraAzul, sprBarraRoja;
    static bool barrasCargadas = false;
    if (!barrasCargadas) {
        texBarraAzul.loadFromFile("assets/barra_azul.png");
        texBarraRoja.loadFromFile("assets/barra_roja.png");
        sprBarraAzul.setTexture(texBarraAzul);
        sprBarraRoja.setTexture(texBarraRoja);
        barrasCargadas = true;
    }

    // Barra de vida azul (izquierda)
    int mA = piezaAzulCombate ? piezaAzulCombate->getVidaMax() : 100;
    float rA = std::max(0.f, (float)vidaAzulCombate / mA);
    // Marco
    sprBarraAzul.setScale(400.f / texBarraAzul.getSize().x, 60.f / texBarraAzul.getSize().y);
    sprBarraAzul.setPosition(10.f, 5.f);
    // Relleno debajo del marco
    sf::RectangleShape rellenoA({ 320.f * rA, 22.f });
    rellenoA.setFillColor(sf::Color(70, 130, 255));
    rellenoA.setPosition(75.f, 24.f);
    window.draw(rellenoA);
    window.draw(sprBarraAzul);

    // Barra de vida roja (derecha)
    int mR = piezaRojaCombate ? piezaRojaCombate->getVidaMax() : 100;
    float rR = std::max(0.f, (float)vidaRojaCombate / mR);
    // Marco (invertido para que el escudo quede a la derecha)
    sprBarraRoja.setScale(-400.f / texBarraRoja.getSize().x, 60.f / texBarraRoja.getSize().y);
    sprBarraRoja.setPosition(1270.f, 5.f);
    // Relleno
    sf::RectangleShape rellenoR({ 320.f * rR, 22.f });
    rellenoR.setFillColor(sf::Color(255, 60, 60));
    rellenoR.setPosition(880.f + 320.f * (1.f - rR), 24.f);
    window.draw(rellenoR);
    window.draw(sprBarraRoja);
    if (fuenteCargada) {
        sf::Text tA(piezaAzulCombate ? piezaAzulCombate->getNombre() : "Azul", fuente, 16);
        tA.setFillColor(sf::Color(150, 200, 255)); tA.setPosition(20.f, 46.f); window.draw(tA);
        sf::Text tR(piezaRojaCombate ? piezaRojaCombate->getNombre() : "Rojo", fuente, 16);
        tR.setFillColor(sf::Color(255, 150, 150)); tR.setPosition(960.f, 46.f); window.draw(tR);
        sf::Text ctrl("Azul: WASD+SPACE  |  Rojo: Flechas+ENTER  |  ESC: volver", fuente, 14);
        ctrl.setFillColor(sf::Color(200, 200, 200)); ctrl.setPosition(330.f, 695.f); window.draw(ctrl);
    }
}

bool Coordinador::salir() { return estado == EstadoJuego::SALIR; }