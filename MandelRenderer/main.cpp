#include <iostream>
#include <string>

int main(int argc, char** argv) {
	if (argc == 2) {
		unsigned int threads = std::stoi(argv[1]);
		for (int i = 0; i < threads; i++) {
			system(("cmd.exe /c start renderer.exe " + std::to_string(i) + " " + argv[1]).c_str());
		}
	}
	if (argc == 3) {
		unsigned int offset = std::stoi(argv[2]);
		unsigned int threads = std::stoi(argv[1]);
		for (int i = 0; i < threads; i++) {
			system(("cmd.exe /c start renderer.exe " + std::to_string(offset + i) + " " + argv[1]).c_str());
		}
	}
	else {
		std::cout << "Syntax: MandelRenderer.exe [threads] ([offset] = 0)" << std::endl;
	}
	return 0;
}