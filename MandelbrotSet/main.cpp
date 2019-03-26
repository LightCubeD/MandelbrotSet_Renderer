#include <stdio.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <Magick++.h>
#include <mpirxx.h>
#include <fstream>

#define PRECISION 1024
#define ITERATIONS 500
#define WIDTH 1270
#define HEIGHT 720	


using namespace Magick;

struct complex {
	complex(std::string Real, std::string Imag) {
		mpf_set_default_prec(PRECISION);
		mpf_init(complex::re);
		mpf_init(complex::im);
		mpf_set_str(complex::re, Real.c_str(), 10);
		mpf_set_str(complex::im, Imag.c_str(), 10);
	}
	complex(mpf_t Real, mpf_t Imag) {
		mpf_set_default_prec(PRECISION);
		mpf_init(complex::re);
		mpf_init(complex::im);
		mpf_set(complex::re, Real);
		mpf_set(complex::im, Imag);
	}
	void set(mpf_t Real, mpf_t Imag) {
		mpf_set(complex::re, Real);
		mpf_set(complex::im, Imag);
	}
	void set(std::string Real, std::string Imag) {
		mpf_set_str(complex::re, Real.c_str(), 10);
		mpf_set_str(complex::im, Imag.c_str(), 10);
	}
	complex() {
		mpf_set_default_prec(PRECISION);
		mpf_init(complex::re);
		mpf_init(complex::im);
	}
	void show() {
		std::cout << "\nREAL: ";
		mpf_out_str(stdout, 10, PRECISION, complex::re);
		std::cout << "\nIMAG: ";
		mpf_out_str(stdout, 10, PRECISION, complex::im);
	}
	mpf_t re, im;
};

void cAdd(complex& o, complex a1, complex a2) {
	mpf_t ima, rea;
	mpf_init(rea);
	mpf_init(ima);
	mpf_add(rea, a1.re, a2.re);
	mpf_add(ima, a1.im, a2.im);
	o.set(rea, ima);
	mpf_clear(ima);
	mpf_clear(rea);
}

void cMult(complex& o, complex a1, complex a2) {
	mpf_t ima, rea, sub1, sub2, sub3, sub4;
	mpf_init(rea);
	mpf_init(ima);
	mpf_init(sub1);
	mpf_init(sub2);
	mpf_init(sub3);
	mpf_init(sub4);
	mpf_mul(sub1, a1.re, a2.re);
	mpf_mul(sub2, a1.im, a2.im);
	mpf_mul(sub3, a1.re, a2.im);
	mpf_mul(sub4, a1.im, a2.re);
	mpf_sub(rea, sub1, sub2);
	mpf_add(ima, sub3, sub4);
	o.set(rea, ima);
	mpf_clear(ima);
	mpf_clear(rea);
	mpf_clear(sub1);
	mpf_clear(sub2);
	mpf_clear(sub3);
	mpf_clear(sub4);
}

void cAbs(mpf_t& o, complex a) {
	mpf_t sum1, sub1, sub2;
	mpf_init(sum1);
	mpf_init(sub1);
	mpf_init(sub2);
	mpf_mul(sub1, a.re, a.re);
	mpf_mul(sub2, a.im, a.im);
	mpf_add(sum1, sub1, sub2);
	mpf_sqrt(o, sum1);
	mpf_clear(sum1);
	mpf_clear(sub1);
	mpf_clear(sub2);
}

double cAbs(complex a) {
	return sqrt(((mpf_get_d(a.re))*(mpf_get_d(a.re)))+((mpf_get_d(a.im))*(mpf_get_d(a.im))));
}

bool cSmallerEquals2(complex a) {
	mpf_t abs;
	mpf_init(abs);
	cAbs(abs, a);
	if (mpf_cmp_ui(abs, (mpir_ui)2.0) <= 0) {
		mpf_clear(abs);
		return true;
	}
	mpf_clear(abs);
	return false;
}


Image image;
complex z;

void makeImg(std::vector<unsigned char> data, std::string title) {
	image.read(WIDTH, HEIGHT, "HSV", CharPixel, &data[0]);
	image.write("Images/" + title + ".png");
}

unsigned int * getIterationsMandelBrot(complex c) {
	unsigned int ret[2];
	unsigned int n = 0;
	z.set("0", "0");

	cAdd(z, z, c);

	while (cSmallerEquals2(z) == true && n < ITERATIONS) {
		cMult(z, z, z);
		cAdd(z, z, c);
		n++;
	}
	ret[0] = n;
	ret[1] = 255 * (n + 1 - log(log2(cAbs(z))));
	return ret;
}

std::string readFile(std::string path) {
	std::string res;
	std::ifstream stream(path);
	while (stream) {
		std::string line;
		std::getline(stream, line);
		res += line;
	}
	return res;
}

int main(int argc, char** argv) {
	unsigned int threads = 1;
	unsigned int toff = 0;
	if (argc == 3) {
		toff = std::stoi(argv[1]);
		threads = std::stoi(argv[2]);
	}
	InitializeMagick(*argv);
	double framesPS = 1.0 / 30.0;
	double length = 1;
	std::vector<unsigned char> data(WIDTH * HEIGHT * 3);
	complex c;
	mpf_t pointX, pointY, scale, offsetXBefore, offsetYBefore, offsetXAfter, offsetYAfter, widthT, scaleV, toffT, scaleFPS;
	mpf_init(pointX);
	mpf_init(pointY);
	mpf_init(scale);
	mpf_init(scaleV);
	mpf_init(scaleFPS);
	mpf_init_set_ui(toffT, toff);
	mpf_init_set_ui(widthT, WIDTH);
	mpf_init_set_d(offsetXBefore, WIDTH / 2);
	mpf_init_set_d(offsetYBefore, HEIGHT / 2);
	std::string xOff = readFile("xOff.txt");
	std::string yOff = readFile("yOff.txt");
	mpf_init_set_str(offsetXAfter, xOff.c_str(), 10);
	mpf_init_set_str(offsetYAfter, yOff.c_str(), 10);
	std::string startScale = readFile("startScale.txt");
	std::string scaleFPSS = readFile("ScaleFPS.txt");
	mpf_set_str(scaleV, startScale.c_str(), 10);
	mpf_set_str(scaleFPS, scaleFPSS.c_str(), 10);
	mpf_t sub1;
	mpf_init_set_ui(sub1, 1);
	mpf_div_ui(sub1, sub1, 60);
	mpf_mul(sub1, sub1, toffT);
	mpf_mul(sub1, sub1, scaleFPS);
	mpf_sub(scaleV, scaleV, sub1);
	mpf_clear(sub1);
	//scaleV -= toff * 3.75 * framesPS;
	for (unsigned int i = toff; i < length / framesPS; i+=threads) {
		//mpf_set_d(scale, WIDTH / scaleV);
		mpf_div(scale, widthT, scaleV);
		//scaleV -= threads * 3.75 * framesPS;
		mpf_t sub1;
		mpf_init_set_ui(sub1, 1);
		mpf_div_ui(sub1, sub1, 60);
		mpf_mul_ui(sub1, sub1, threads);
		mpf_mul(sub1, sub1, scaleFPS);
		mpf_sub(scaleV, scaleV, sub1);
		mpf_clear(sub1);

		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				mpf_set_ui(pointX, x);
				mpf_set_ui(pointY, y);
				mpf_sub(pointX, pointX, offsetXBefore);
				mpf_sub(pointY, pointY, offsetYBefore);
				mpf_div(pointX, pointX, scale);
				mpf_div(pointY, pointY, scale);
				mpf_sub(pointX, pointX, offsetXAfter);
				mpf_add(pointY, pointY, offsetYAfter);
				c.set(pointX, pointY);
				unsigned int i = (x + (y * WIDTH)) * 3;
				unsigned int* it = getIterationsMandelBrot(c);
				if (it[0] == ITERATIONS) {
					data[i] = 0;
					data[i + 1] = 0;
					data[i + 2] = 0;
				}
				else {
					data[i] = it[1];
					data[i + 1] = 255;
					data[i + 2] = 255;
				}
			}
			double p, pbs;
			pbs = 100.0 * (y + (i * HEIGHT)) / (HEIGHT * (length / framesPS));
			p = 100.0 * y / HEIGHT;
			std::cout << "ABS: " << std::to_string(pbs) << "%     " << std::to_string(p) << "%  I: " << i << std::endl;
		}
		makeImg(data, std::to_string(i));
	}

	mpf_clear(pointX);
	mpf_clear(pointY);
	mpf_clear(scale);
	mpf_clear(widthT);
	mpf_clear(scaleV);
	mpf_clear(toffT);
	mpf_clear(scaleFPS);
	mpf_clear(offsetXBefore);
	mpf_clear(offsetYBefore);
	mpf_clear(offsetXAfter);
	mpf_clear(offsetYAfter);
	std::cout << "DONE\n";
	std::cin.get();
	return 0;
}	