#ifndef __VIRUS_GENEALOGY__
#define __VIRUS_GENEALOGY__

#include <vector>
#include <memory>
#include <map>


// Pomocniczo, zeby sprawdzic czy sie kompiluje
// Do usuniecia po skonczeniu
#include <string>


class Virus {
public:
	typedef typename std::string id_type;
	Virus(id_type const &_id) : id(_id) {
	}
	id_type get_id() const {
		return id;
	}
private:
	id_type id;
};
// koniec pomocniczych deklaracji

/* WAZNE INFO Z TRESCI
 * wszystkie metody klasy VirusGenealogy powinny gwarantować silną odporność
   na wyjątki, a tam, gdzie to jest możliwe i pożądane, powinny być no-throw;
 * wyszukiwanie wirusów powinno być szybsze niż liniowe.
 * zarządzanie pamięcią powinno być zrealizowane za pomocą sprytnych wskaźników
   z biblioteki standardowej.
*/

class VirusNotFound : public std::exception {
	virtual const char* what() const throw() {
		return "VirusNotFound";
	}
};
class VirusAlreadyCreated : public std::exception {
	virtual const char* what() const throw() {
		return "VirusAlreadyCreated";
	}
};
class TriedToRemoveStemVirus : public std::exception {
	virtual const char* what() const throw() {
		return "TriedToRemoveStemVirus";
	}
};


//@TODO : ktore smart-pointery uzyc w konkretnym przypadku?
//@TODO : wyjatki - nie ma poki co zadnej odpornosci na wyjatki!
//@TODO : sprawdzanie czy wirus istnieje i wyrzucanie wyjatku, opakowac w 1 funkcje prywatna
// 		  zamiast copy-pasty
template <class Virus>
class VirusGenealogy {
	using id_type = typename Virus::id_type;

public:
	VirusGenealogy(const VirusGenealogy &) = delete;
	VirusGenealogy & operator=(const VirusGenealogy &) = delete;

	VirusGenealogy(id_type const &stem_id) {
		// stworz wierzcholek z wirusem macierzystym i dodaj do mapy
		_stem = std::make_shared<node>(stem_id);
		_all_nodes.insert(make_pair(stem_id, _stem));
	}

	id_type get_stem_id() const {
		return _stem._virus->get_id();
	}
	bool exists(id_type const &id) const {
		return _all_nodes.find(id) != _all_nodes.end();
	}
	//@TODO : straszny copypaste z get_parents! Wydzielic do jednej funkcji
	std::vector<id_type> get_children(id_type const &id) const {
		// 1. sprawdz czy istnieje wirus o identyfikatorze id, wpp rzuc wyjatek
		auto it = _all_nodes.find(id);
		if (it == _all_nodes.end())
			throw new VirusNotFound();
		// stworz wektor w ktorym zwrocone zostana id nastepnikow
		std::vector<id_type> children;
		// dla kazdego dziecka wezla, na ktory pokazuje iterator it wrzuc do wektora
		// identyfikator dziecka tego wezla
		for (auto &node_ptr : it->_children)
			children.push_back(node_ptr->_virus->_get_id());
		return children;
	}
	std::vector<id_type> get_parents(id_type const &id) const {
		// analogicznie jak w get_children
		auto it = _all_nodes.find(id);
		if (it == _all_nodes.end())
			throw new VirusNotFound();
		std::vector<id_type> parents;
		for (auto &node_ptr : it->_parents)
			parents.push_back(node_ptr->_virus->_get_id());
		return parents;
	}
	Virus& operator[](id_type const & id) const {
		auto it = _all_nodes.find(id);
		if (it == _all_nodes.end())
			throw new VirusNotFound();
		return *it;
	}
	void create(id_type const & id, id_type const & parent_id);
	void create(id_type const & id, std::vector<id_type> const & parent_ids);
	void connect(id_type const & child_id, id_type const & parent_id);
	void remove(id_type const & id);

private:
	// struktura na przechowywanie wierzcholkow grafu genealogii -> moze klasa z publicznymi getterami setteram?
	struct node {
		//http://stackoverflow.com/questions/27348396/smart-pointers-for-graph-representation-vertex-neighbors-in-c11
		std::unique_ptr<Virus> _virus;
		std::vector<std::shared_ptr<node> > _children;
		std::vector<std::weak_ptr<node> > _parents;
		node(id_type const & stem_id) {
			_virus = std::make_unique<Virus>(stem_id);
		}
	};
	// mapa wszystkich potomkow wirusa
	std::map<id_type, std::weak_ptr<node> > _all_nodes;
	// wirus macierzysty
	std::shared_ptr<node> _stem;

};

#endif /* __VIRUS_GENEALOGY__ */