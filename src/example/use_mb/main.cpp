#include "IMB.h"

#include <iostream>
#include <stdio.h>
#include <thread>

int main(void) {
	std::string path_config = "example.ini";
	std::unique_ptr<mb::IMB> mb = mb::IMB::create(path_config);
	bool result = mb->start();
	mb::IMB::IData* g = mb->getData(200,4);
	if (g == nullptr) std::cout << "GGWP" << std::endl;
	g->getBit();

	while(true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	// g.getBit();
	
	// mb::MB mb(path_config);
	// mb::MB::Data data;
	// data.getBit();
	// mb.start();
	// mb::IMB::IData data;
	// mb::Data d;
	// g.getBit();

	// mb::config::Config config(path_config);
	// mb::data::DataManager dm(config);
	// bool result = mb.start();
	// if (result) std::cout << "Mb started " << std::endl;
	
	// mb::Data data = mb->getCoilData(1);
	// data.getBit();
	// mb->getCoil(123,1);
	// mb->getData(123,3,1);

	return 0;
}

// class {
// 	public:
// 		errors = mb.getCoil(1);
// 		// mb->getData("asd",3,1);
	// Напрямую
	// mb->getData("")

// 	private;
// 	mb::Data errors;
// 	mb::Data warnings;

// }