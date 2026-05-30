#include "Coordinador.h"
#include <cmath>
#include <algorithm>
#include <SFML/Audio.hpp>

static constexpr float TAM_CASILLA = 70.f;
static constexpr float OFFSET_X = 100.f;
static constexpr float OFFSET_Y = 50.f;

Coordinador::Coordinador()
    : estado(EstadoJuego::MENU), menu(1280, 720)
{
    fuenteCargada = fuente.loadFromFile("assets/supercell-magic.ttf");
    if (bufferMuerte.loadFromFile("assets/jija.ogg"))
        sonidoMuerte.setBuffer(bufferMuerte);
    if (bufferMenu.loadFromFile("assets/clash-royale-start-up-sound.ogg"))
        sonidoMenu.setBuffer(bufferMenu);
    sonidoMenu.play();
    texObstaculo.loadFromFile("assets/Tronco_Obstaculo.png");
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
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::H && selFila != -1) {
                tablero.ejecutarHechizo(static_cast<int>(IdHechizo::Curar), selFila, selCol);
                selFila = selCol = -1;
            }
            if (event.key.code == sf::Keyboard::E && selFila != -1) {
                tablero.ejecutarHechizo(static_cast<int>(IdHechizo::Encarcelar), selFila, selCol);
                selFila = selCol = -1;
            }
            if (event.key.code == sf::Keyboard::R) {
                tablero.ejecutarHechizo(static_cast<int>(IdHechizo::Revivir), 0, 0);
            }
            if (event.key.code == sf::Keyboard::T && selFila != -1) {
                modoTeleport = true;
                teleportOrigenFila = selFila;
                teleportOrigenCol = selCol;
                selFila = selCol = -1;
            }
        }
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
    if (estado == EstadoJuego::FIN_PARTIDA) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape)
                estado = EstadoJuego::MENU;
            if (event.key.code == sf::Keyboard::R)
                reiniciar();
        }
    }
}

void Coordinador::procesarClickTablero(int px, int py) {
    int col = static_cast<int>((px - OFFSET_X) / TAM_CASILLA);
    int fil = static_cast<int>((py - OFFSET_Y) / TAM_CASILLA);

    if (col < 0 || col >= Tablero::TAM || fil < 0 || fil >= Tablero::TAM) {
        selFila = selCol = -1;
        modoTeleport = false;
        return;
    }

    if (modoTeleport) {
        tablero.ejecutarHechizo(
            static_cast<int>(IdHechizo::Teleportar),
            teleportOrigenFila, teleportOrigenCol,
            fil, col
        );
        modoTeleport = false;
        teleportOrigenFila = teleportOrigenCol = -1;
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
    velAzul = 5.f + (piezaAzulCombate ? piezaAzulCombate->getVelocidad() * 1.5f : 0.f);
    velRoja = 5.f + (piezaRojaCombate ? piezaRojaCombate->getVelocidad() * 1.5f : 0.f);
    recargaAzul = recargaRoja = 0.f;
    proyectiles.clear();
    relojCombate.restart();
 
    obstaculos.clear();
    // Crear un par de obstáculos en el centro del mapa
    Obstaculo obs1, obs2;
    obs1.sprite.setTexture(texObstaculo);
    // Ajusta la posición
    obs1.sprite.setScale(0.12f, 0.12f);

    // Lo colocamos en la parte superior central de la arena
    obs1.sprite.setPosition(500.f, 150.f);

    // Calculamos la caja de colisión DESPUÉS de escalar para que el choque sea exacto
    obs1.hitbox = obs1.sprite.getGlobalBounds();

    // Segundo tronco (Abajo)
    obs2.sprite.setTexture(texObstaculo);
    obs2.sprite.setScale(0.12f, 0.12f); // Usa la misma escala que el primero
    obs2.sprite.setPosition(650.f, 480.f); // Lo colocamos en la parte inferior
    obs2.hitbox = obs2.sprite.getGlobalBounds();

    // Los añadimos a la arena
    obstaculos.push_back(obs1);
    obstaculos.push_back(obs2);
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
            ganoAzul = true;
            estado = EstadoJuego::FIN_PARTIDA;
        }
        else if (res == ResultadoVictoria::GanaRojo) {
            ganadorTexto = "GANA EL EQUIPO ROJO";
            ganoAzul = false;
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

    sf::Vector2f posAzulAnt = posAzul;
    sf::Vector2f posRojaAnt = posRoja;

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

    // COLISIONES DE LOS JUGADORES CON LOS OBSTÁCULOS
    sf::FloatRect hitboxAzul(posAzul.x - 24, posAzul.y - 24, 48, 48);
    for (const auto& obs : obstaculos) {
        if (hitboxAzul.intersects(obs.hitbox)) posAzul = posAzulAnt;
    }

    sf::FloatRect hitboxRoja(posRoja.x - 24, posRoja.y - 24, 48, 48);
    for (const auto& obs : obstaculos) {
        if (hitboxRoja.intersects(obs.hitbox)) posRoja = posRojaAnt;
    }

    // COLISIONES ENTRE JUGADORES (Física sólida)
    float dxJugadores = posRoja.x - posAzul.x;
    float dyJugadores = posRoja.y - posAzul.y;
    float distJugadores = std::sqrt(dxJugadores * dxJugadores + dyJugadores * dyJugadores);

    if (distJugadores < 48.f) {
        posAzul = posAzulAnt;
        posRoja = posRojaAnt;
    }

    recargaAzul -= dt; recargaRoja -= dt;

    // HELPER MATEMÁTICO (Descomentado porque las explosiones lo necesitan)
    auto distancia = [](sf::Vector2f a, sf::Vector2f b) {
        float dx = a.x - b.x, dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
        };

    float intA = std::max(0.2f, 1.5f - piezaAzulCombate->getVelocidadAtaque() * 0.13f);
    float intR = std::max(0.2f, 1.5f - piezaRojaCombate->getVelocidadAtaque() * 0.13f);

    // ── ATAQUES EQUIPO AZUL ──
    if (piezaAzulCombate->getArma() == TipoArma::CuerpoACuerpo) {
        // Solo ataca con la F si es cuerpo a cuerpo
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F) && recargaAzul <= 0.f) {
            if (distancia(posAzul, posRoja) < 80.f) {
                vidaRojaCombate = std::max(0, vidaRojaCombate - piezaAzulCombate->getFuerzaAtaque());
            }
            recargaAzul = intA;
        }
    }
    else {
        // Dispara con Espacio si tiene arma a distancia
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && recargaAzul <= 0.f) {
            dispararAzul(); recargaAzul = intA;
        }
    }

    // ── ATAQUES EQUIPO ROJO ──
    if (piezaRojaCombate->getArma() == TipoArma::CuerpoACuerpo) {
        // Solo ataca con Shift Derecho si es cuerpo a cuerpo
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift) && recargaRoja <= 0.f) {
            if (distancia(posAzul, posRoja) < 80.f) {
                vidaAzulCombate = std::max(0, vidaAzulCombate - piezaRojaCombate->getFuerzaAtaque());
            }
            recargaRoja = intR;
        }
    }
    else {
        // Dispara con Enter si tiene arma a distancia
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return) && recargaRoja <= 0.f) {
            dispararRoja(); recargaRoja = intR;
        }
    }

    // ── Mover proyectiles y aplicar daño ──
    for (auto& p : proyectiles) {
        if (!p.activo) continue;
        p.forma.move(p.vel);
        auto pos = p.forma.getPosition();

        // Fuera de pantalla
        if (pos.x < 0 || pos.x > 1280 || pos.y < 0 || pos.y > 720) {
            p.activo = false; continue;
        }

        // Colisión con obstáculos (solo si NO es magia)
        if (!p.atraviesaObstaculos) {
            for (const auto& obs : obstaculos) {
                if (p.forma.getGlobalBounds().intersects(obs.hitbox)) {
                    p.activo = false; break;
                }
            }
            if (!p.activo) continue;
        }

        sf::FloatRect pb = p.forma.getGlobalBounds();

        if (p.esAzul) {
            sf::FloatRect hitR(posRoja.x - 24, posRoja.y - 24, 48, 48);
            if (pb.intersects(hitR)) {
                if (p.esExplosion) {
                    vidaRojaCombate = std::max(0, vidaRojaCombate - p.danio);
                    if (distancia(posAzul, posRoja) < p.radioExplosion)
                        vidaAzulCombate = std::max(0, vidaAzulCombate - p.danio / 3);
                }
                else {
                    vidaRojaCombate = std::max(0, vidaRojaCombate - p.danio);
                }
                p.activo = false;
            }
        }
        else {
            sf::FloatRect hitA(posAzul.x - 24, posAzul.y - 24, 48, 48);
            if (pb.intersects(hitA)) {
                if (p.esExplosion) {
                    vidaAzulCombate = std::max(0, vidaAzulCombate - p.danio);
                    if (distancia(posAzul, posRoja) < p.radioExplosion)
                        vidaRojaCombate = std::max(0, vidaRojaCombate - p.danio / 3);
                }
                else {
                    vidaAzulCombate = std::max(0, vidaAzulCombate - p.danio);
                }
                p.activo = false;
            }
        }
    }

    proyectiles.erase(std::remove_if(proyectiles.begin(), proyectiles.end(),
        [](const Proyectil& p) { return !p.activo; }), proyectiles.end());

    if (vidaAzulCombate <= 0 || vidaRojaCombate <= 0) {
        sonidoMuerte.play();

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
// ── Helper interno para crear un proyectil base
static Coordinador::Proyectil crearProyectil(
    sf::Vector2f origen, sf::Vector2f destino,
    int fuerzaAtaque, int alcance, bool esAzul,
    TipoArma arma)
{
    sf::Vector2f dir = destino - origen;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 1.f) dir = { 1.f, 0.f };
    else dir /= len;

    Coordinador::Proyectil p;
    p.esAzul = esAzul;
    p.activo = true;
    p.atraviesaObstaculos = false;
    p.esExplosion = false;
    p.radioExplosion = 0.f;

    switch (arma) {
    case TipoArma::CuerpoACuerpo:
        // No genera proyectil, se maneja en actualizarCombate
        p.activo = false;
        break;

    case TipoArma::Proyectil:
        p.forma.setRadius(6.f);
        p.forma.setOrigin(6.f, 6.f);
        p.forma.setFillColor(esAzul ? sf::Color(100, 220, 255) : sf::Color(255, 160, 60));
        p.forma.setPosition(origen);
        p.vel = dir * (320.f + alcance * 15.f) / 60.f;
        p.danio = fuerzaAtaque;
        break;

    case TipoArma::Magia:
        p.forma.setRadius(9.f);
        p.forma.setOrigin(9.f, 9.f);
        p.forma.setFillColor(esAzul ? sf::Color(180, 100, 255) : sf::Color(100, 255, 180));
        p.forma.setPosition(origen);
        p.vel = dir * (200.f + alcance * 10.f) / 60.f;  // más lento
        p.danio = fuerzaAtaque;
        p.atraviesaObstaculos = true;  // la magia no rebota en obstáculos
        break;

    case TipoArma::ExplosionArea:
        p.forma.setRadius(7.f);
        p.forma.setOrigin(7.f, 7.f);
        p.forma.setFillColor(sf::Color(255, 120, 0));
        p.forma.setPosition(origen);
        p.vel = dir * (260.f + alcance * 12.f) / 60.f;
        p.danio = fuerzaAtaque;
        p.esExplosion = true;
        p.radioExplosion = 80.f;
        break;
    }
    return p;
}

void Coordinador::dispararAzul() {
    if (!piezaAzulCombate) return;
    TipoArma arma = piezaAzulCombate->getArma();
    if (arma == TipoArma::CuerpoACuerpo) return; // sin proyectil
    auto p = crearProyectil(posAzul, posRoja,
        piezaAzulCombate->getFuerzaAtaque(),
        piezaAzulCombate->getAlcanceAtaque(),
        true, arma);
    if (p.activo) proyectiles.push_back(p);
}

void Coordinador::dispararRoja() {
    if (!piezaRojaCombate) return;
    TipoArma arma = piezaRojaCombate->getArma();
    if (arma == TipoArma::CuerpoACuerpo) return;
    auto p = crearProyectil(posRoja, posAzul,
        piezaRojaCombate->getFuerzaAtaque(),
        piezaRojaCombate->getAlcanceAtaque(),
        false, arma);
    if (p.activo) proyectiles.push_back(p);
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

    if (estado == EstadoJuego::FIN_PARTIDA) {
        static sf::Texture texVictAzul, texVictRojo;
        static sf::Sprite sprVict;
        static bool victCargada = false;
        if (!victCargada) {
            texVictAzul.loadFromFile("assets/victoria_azul.png");
            texVictRojo.loadFromFile("assets/victoria_rojo.png");
            victCargada = true;
        }
        sprVict.setTexture(ganoAzul ? texVictAzul : texVictRojo);
        sprVict.setScale(
            1280.f / sprVict.getTexture()->getSize().x,
            720.f / sprVict.getTexture()->getSize().y
        );
        window.draw(sprVict);
        return;
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
 
    for (const auto& obs : obstaculos) {
        window.draw(obs.sprite);
    }

    // Cargar las texturas de los personajes solo una vez 
    static std::map<std::string, sf::Texture> texturasCombate;
    static bool texCargadas = false;
    if (!texCargadas) {
        texturasCombate["Caballero"].loadFromFile("assets/caballero.png");
        texturasCombate["Caballero_oscuro"].loadFromFile("assets/principe oscuro.png");
        texturasCombate["Golem"].loadFromFile("assets/golem.png");
        texturasCombate["PEKKA"].loadFromFile("assets/pekka.png");
        texturasCombate["Dragon"].loadFromFile("assets/dragon.png");
        texturasCombate["Dragon_infernal"].loadFromFile("assets/dragon infernal.png");
        texturasCombate["Arqueras"].loadFromFile("assets/arquera.png");
        texturasCombate["Reina_arquera"].loadFromFile("assets/reina arquera.png");
        texturasCombate["Valkiria"].loadFromFile("assets/valkiria.png");
        texturasCombate["Bandida"].loadFromFile("assets/bandida.png");
        texturasCombate["Curandera"].loadFromFile("assets/curandera.png");
        texturasCombate["Murcielago"].loadFromFile("assets/fenix.png");
        texturasCombate["Esbirro"].loadFromFile("assets/esbirro.png");
        texturasCombate["Dragon_electrico"].loadFromFile("assets/dragon electrico.png");
        texturasCombate["Mago"].loadFromFile("assets/mago.png");
        texturasCombate["Bruja"].loadFromFile("assets/bruja.png");
        texCargadas = true;
    }

    // Dibujar el personaje AZUL
    if (piezaAzulCombate) {
        std::string nombreAzul = piezaAzulCombate->getNombre();
        if (texturasCombate.count(nombreAzul) > 0) {
            sf::Sprite sprAzul(texturasCombate[nombreAzul]);
            // Centramos el origen en la mitad de la imagen para que coincida con la hitbox y dispare del centro
            sprAzul.setOrigin(sprAzul.getTexture()->getSize().x / 2.f, sprAzul.getTexture()->getSize().y / 2.f);
            sprAzul.setPosition(posAzul);

            // Calculamos una escala para que midan unos 80 píxeles en la arena
            float escalaA = 80.f / texturasCombate[nombreAzul].getSize().x;
            sprAzul.setScale(escalaA, escalaA);
            window.draw(sprAzul);
        }
    }

    // Dibujar el personaje ROJO
    if (piezaRojaCombate) {
        std::string nombreRojo = piezaRojaCombate->getNombre();
        if (texturasCombate.count(nombreRojo) > 0) {
            sf::Sprite sprRojo(texturasCombate[nombreRojo]);
            sprRojo.setOrigin(sprRojo.getTexture()->getSize().x / 2.f, sprRojo.getTexture()->getSize().y / 2.f);
            sprRojo.setPosition(posRoja);

            float escalaR = 80.f / texturasCombate[nombreRojo].getSize().x;
            // Le ponemos un menos (-) en la escala X para voltear la imagen 
            // y que el equipo rojo siempre mire hacia la izquierda (hacia el enemigo)
            sprRojo.setScale(-escalaR, escalaR);
            window.draw(sprRojo);
        }
    }

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
    int mA = piezaAzulCombate ? piezaAzulCombate->getVidaMax() : 100;
    float rA = std::max(0.f, (float)vidaAzulCombate / mA);
    int mR = piezaRojaCombate ? piezaRojaCombate->getVidaMax() : 100;
    float rR = std::max(0.f, (float)vidaRojaCombate / mR);
    // Barra azul
    float altoBarra = 120.f;
    float anchoBarra = 854.f * (altoBarra / 292.f); // mantiene proporción
    sprBarraAzul.setScale(anchoBarra / texBarraAzul.getSize().x, altoBarra / texBarraAzul.getSize().y);
    sprBarraAzul.setPosition(10.f, 3.f);
    sf::RectangleShape rellenoA({ (anchoBarra - 120.f) * rA, 35.f });
    rellenoA.setFillColor(sf::Color(70, 130, 255));
    rellenoA.setPosition(110.f, 41.f);
    window.draw(rellenoA);
    window.draw(sprBarraAzul);

    // Barra roja
    float altoBarra2 = 120.f;
    float anchoBarra2 = 857.f * (altoBarra2 / 291.f);
    sprBarraRoja.setScale(-anchoBarra2 / texBarraRoja.getSize().x, altoBarra2 / texBarraRoja.getSize().y);
    sprBarraRoja.setPosition(1250.f, 3.f);
    float xBarraRoja = 1270.f - anchoBarra2 + 110.f;
    sf::RectangleShape rellenoR({ (anchoBarra2 - 80.f) * rR, 31.f });
    rellenoR.setFillColor(sf::Color(255, 60, 60));
   
    rellenoR.setPosition(xBarraRoja + (anchoBarra2 - 150.f) * (1.f - rR)-100.f, 42.f);
    window.draw(rellenoR);
    window.draw(sprBarraRoja);
    if (fuenteCargada) {
        sf::Text tA(piezaAzulCombate ? piezaAzulCombate->getNombre() : "Azul", fuente, 16);
        tA.setFillColor(sf::Color(150, 200, 255)); tA.setPosition(100.f, 100.f); window.draw(tA);
        sf::Text tR(piezaRojaCombate ? piezaRojaCombate->getNombre() : "Rojo", fuente, 16);
        tR.setFillColor(sf::Color(255, 150, 150)); tR.setPosition(1350.f - anchoBarra2 + 20.f, 100.f); window.draw(tR);
        sf::Text ctrl("Azul: WASD+SPACE  |  Rojo: Flechas+ENTER  |  ESC: volver", fuente, 14);
        ctrl.setFillColor(sf::Color(200, 200, 200)); ctrl.setPosition(330.f, 695.f); window.draw(ctrl);
    }
}   
void Coordinador::reiniciar() {
    tablero.inicializa();
    piezaAzulCombate = piezaRojaCombate = nullptr;
    proyectiles.clear();
    obstaculos.clear();
    selFila = selCol = -1;
    vidaAzulCombate = vidaRojaCombate = 0;
    estado = EstadoJuego::TABLERO;
}


bool Coordinador::salir() { return estado == EstadoJuego::SALIR; }