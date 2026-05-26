#include "Menu.h"
#include <iostream>

Menu::Menu(float width, float height)
    : anchoVentana(width), altoVentana(height)
{
}

void Menu::ajustarFondo(const sf::Texture& texture) {
    sprBackground.setTexture(texture);
    sprBackground.setScale(
        anchoVentana / texture.getSize().x,
        altoVentana / texture.getSize().y
    );
    sprBackground.setPosition(0.f, 0.f);
}

void Menu::configurarMenu() {
    menuOptions.clear();
    std::vector<std::string> names = { "JUGAR", "CONTROLES", "SALIR" };

    float positionsX = 664.f;
    float positionsY[] = { 353.f, 458.f, 570.f };

    for (int i = 0; i < (int)names.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(names[i]);
        text.setCharacterSize(45);
        text.setFillColor(sf::Color(255, 200, 0));
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(4);

        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(
            textRect.left + textRect.width / 2.f,
            textRect.top + textRect.height / 2.f
        );
        text.setPosition(positionsX, positionsY[i]);

        menuOptions.push_back(text);
    }
}

bool Menu::cargarRecursos() {
    if (!font.loadFromFile("assets/supercell-magic.ttf")) {
        return false;
    }
    if (!texFondoNormal.loadFromFile("assets/menu_normal.png")) {
        return false;
    }

    if (texInstrucciones.loadFromFile("assets/instrucciones.png")) {
        sprInstrucciones.setTexture(texInstrucciones);
        sprInstrucciones.setScale(
            anchoVentana / texInstrucciones.getSize().x,
            altoVentana / texInstrucciones.getSize().y
        );
        sprInstrucciones.setPosition(0.f, 0.f);
        texInstruccionesCargada = true;
    }

    ajustarFondo(texFondoNormal);
    configurarMenu();
    return true;
}

void Menu::actualizar(sf::RenderWindow& window) {
    if (mostrandoInstrucciones) return;

    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    for (auto& text : menuOptions) {
        if (text.getGlobalBounds().contains(mouse))
            text.setFillColor(sf::Color(255, 230, 50));  // dorado brillante hover
        else
            text.setFillColor(sf::Color(255, 200, 0));   // dorado normal
    }
}

EstadoMenu Menu::procesarEventos(sf::RenderWindow& window, sf::Event& event) {
    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Escape)
    {
        if (mostrandoInstrucciones) {
            mostrandoInstrucciones = false;
            return EstadoMenu::MENU_PRINCIPAL;
        }
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        if (mostrandoInstrucciones) {
            mostrandoInstrucciones = false;
            return EstadoMenu::MENU_PRINCIPAL;
        }

        sf::Vector2f mouse = window.mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
        );

        for (int i = 0; i < (int)menuOptions.size(); ++i) {
            if (menuOptions[i].getGlobalBounds().contains(mouse)) {
                if (i == 0) return EstadoMenu::JUGANDO;
                if (i == 1) {
                    mostrandoInstrucciones = true;
                    return EstadoMenu::MENU_PRINCIPAL;
                }
                if (i == 2) return EstadoMenu::SALIR;
            }
        }
    }

    return EstadoMenu::MENU_PRINCIPAL;
}

void Menu::dibujar(sf::RenderWindow& window) {
    if (mostrandoInstrucciones) {
        if (texInstruccionesCargada)
            window.draw(sprInstrucciones);
        return;
    }

    window.draw(sprBackground);
    for (const auto& text : menuOptions)
        window.draw(text);
}