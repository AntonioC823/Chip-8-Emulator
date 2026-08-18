#include "Chip8.h"
#include <SFML/Audio.hpp>
#include <cmath>
#include <cstdint>
#include <vector>

constexpr int CYCLES_PER_FRAME = 10;
constexpr unsigned int SAMPLE_RATE = 44100;
constexpr float FREQUENCY = 440.0f;
constexpr float DURATION = 0.1f;
constexpr double PI = 3.1415926535897;
constexpr std::int16_t AMPLITUDE = 3000;


int main()
{
	Chip8 chip8;

	chip8.loadROM("PONG.ch8");

	std::vector<std::int16_t> samples(static_cast<std::size_t>(SAMPLE_RATE * DURATION));

	for (std::size_t i = 0; i < samples.size(); i++)
	{
		double time = static_cast<double>(i) / SAMPLE_RATE;

		samples[i] = static_cast<std::int16_t>(AMPLITUDE * std::sin(2.0 * PI * FREQUENCY * time));
	}
	sf::SoundBuffer soundBuffer;

	if (!soundBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE, { sf::SoundChannel::Mono }))
	{
		return 1;
	}

	sf::Sound beep(soundBuffer);
	beep.setLooping(true);

	// Create the main window
	sf::RenderWindow window(sf::VideoMode({ 640, 320 }), "Chip-8");
	window.setFramerateLimit(60);

	// Start loop
	while (window.isOpen())
	{
		// Process events
		while (const std::optional event = window.pollEvent())
		{
			// Close window
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}

			// Key pressed
			if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
			{
				switch (keyPressed->scancode)
				{
				case sf::Keyboard::Scancode::Num1:
					chip8.pressKey(0x1);
					break;
				case sf::Keyboard::Scancode::Num2:
					chip8.pressKey(0x2);
					break;
				case sf::Keyboard::Scancode::Num3:
					chip8.pressKey(0x3);
					break;
				case sf::Keyboard::Scancode::Num4:
					chip8.pressKey(0xC);
					break;
				case sf::Keyboard::Scancode::Q:
					chip8.pressKey(0x4);
					break;
				case sf::Keyboard::Scancode::W:
					chip8.pressKey(0x5);
					break;
				case sf::Keyboard::Scancode::E:
					chip8.pressKey(0x6);
					break;
				case sf::Keyboard::Scancode::R:
					chip8.pressKey(0xD);
					break;
				case sf::Keyboard::Scancode::A:
					chip8.pressKey(0x7);
					break;
				case sf::Keyboard::Scancode::S:
					chip8.pressKey(0x8);
					break;
				case sf::Keyboard::Scancode::D:
					chip8.pressKey(0x9);
					break;
				case sf::Keyboard::Scancode::F:
					chip8.pressKey(0xE);
					break;
				case sf::Keyboard::Scancode::Z:
					chip8.pressKey(0xA);
					break;
				case sf::Keyboard::Scancode::X:
					chip8.pressKey(0x0);
					break;
				case sf::Keyboard::Scancode::C:
					chip8.pressKey(0xB);
					break;
				case sf::Keyboard::Scancode::V:
					chip8.pressKey(0xF);
					break;
				default:
					break;
				}
			}

			// Key released
			if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
			{
				switch (keyReleased->scancode)
				{
				case sf::Keyboard::Scancode::Num1:
					chip8.releaseKey(0x1);
					break;
				case sf::Keyboard::Scancode::Num2:
					chip8.releaseKey(0x2);
					break;
				case sf::Keyboard::Scancode::Num3:
					chip8.releaseKey(0x3);
					break;
				case sf::Keyboard::Scancode::Num4:
					chip8.releaseKey(0xC);
					break;
				case sf::Keyboard::Scancode::Q:
					chip8.releaseKey(0x4);
					break;
				case sf::Keyboard::Scancode::W:
					chip8.releaseKey(0x5);
					break;
				case sf::Keyboard::Scancode::E:
					chip8.releaseKey(0x6);
					break;
				case sf::Keyboard::Scancode::R:
					chip8.releaseKey(0xD);
					break;
				case sf::Keyboard::Scancode::A:
					chip8.releaseKey(0x7);
					break;
				case sf::Keyboard::Scancode::S:
					chip8.releaseKey(0x8);
					break;
				case sf::Keyboard::Scancode::D:
					chip8.releaseKey(0x9);
					break;
				case sf::Keyboard::Scancode::F:
					chip8.releaseKey(0xE);
					break;
				case sf::Keyboard::Scancode::Z:
					chip8.releaseKey(0xA);
					break;
				case sf::Keyboard::Scancode::X:
					chip8.releaseKey(0x0);
					break;
				case sf::Keyboard::Scancode::C:
					chip8.releaseKey(0xB);
					break;
				case sf::Keyboard::Scancode::V:
					chip8.releaseKey(0xF);
					break;
				default:
					break;
				}
			}
		}

		// Updates
		for (int i = 0; i < CYCLES_PER_FRAME; i++)
		{
			chip8.cycle();
		}

		if (chip8.isSoundActive())
		{
			if (beep.getStatus() != sf::SoundSource::Status::Playing)
			{
				beep.play();
			}
		}
		else
		{
			if (beep.getStatus() == sf::SoundSource::Status::Playing)
			{
				beep.stop();
			}
		}

		// Clear screen
		window.clear();

		// Draw and update window
		const std::vector<sf::RectangleShape> pixels = chip8.printDisplay();

		for (std::size_t i = 0; i < pixels.size(); i++)
		{
			window.draw(pixels[i]);
		}

		window.display();
	}

	return 0;
}