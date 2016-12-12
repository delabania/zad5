#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <memory>


// Pomocniczy plik zeby ogarnac jak to ma dzialac bez paskudzenia we wlasciwym kodzie

// http://stackoverflow.com/questions/28733385/c11-vector-of-smart-pointer

class Virus {
public:
	Virus(std::string const &_id) : id(_id) {
	}
	std::string get_id() const {
		return id;
	}
private:
	std::string id;
};



class VirusGenealogy {
private:

public:
};











int main() {

}
