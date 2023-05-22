#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

int main(int argc, char *argv[])
{
	if (argc != 3) {
		std::cerr << "Usage: "
			  << argv[0]
			  << " INPUT_FILE OUTPUT_FILE"
			  << std::endl;
		return 1;
	}

	std::ifstream input(argv[1]);
	std::ofstream output(argv[2]);

	if (!input || !output) {
		std::cerr << "Cannot open file.\n";
		return 1;
	}

	output << "import turtle\n";
	output << "t = turtle.Turtle()\n";
	output << "t.speed(100)\n\n";

	std::string line;

	while (std::getline(input, line)) {
		std::istringstream iss(line);
		std::string command;
		int value;
		iss >> command;

		if (command == "Recule") {
			iss >> value;
			output << "t.backward(" << value << ")\n";
		} else if (command == "Avance") {
			iss >> value;
			output << "t.forward(" << value << ")\n";
		} else if (command == "Tourne") {
			std::string preposition, direction;
			iss >> direction >> preposition >> value;

			if (preposition != "de" || (direction != "droite" && direction != "gauche")) {
				std::cerr << "Incorrect command format: " << line << '\n';
				continue;
			}

			if (direction == "droite")
				output << "t.right(" << value << ")\n";
			else if (direction == "gauche")
				output << "t.left(" << value << ")\n";
		} else {
			std::cerr << "Unknown command: " << command << '\n';
		}
	}

	output << "turtle.done()\n";

	return 0;
}
