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
		// wyszukaj wezel o numerze id, gdy nie istnieje rzuc wyjatek => prywatna funkcja get_node
		auto current = get_node(id);
		std::vector<id_type> children;
		// current->_children to wektor <weak_ptr> nastepnikow (dzieci) wezla current
		for (auto & node_ptr : current->_children)
			children.push_back(node_ptr->_virus->_get_id());
		return children;
	}


	std::vector<id_type> get_parents(id_type const &id) const {
		// analogicznie jak w get_children
		auto current = get_node(id);
		std::vector<id_type> parents;
		// current->_parent to wektor <weak_ptr> nastepnikow (dzieci) wezla current
		for (auto & node_ptr : current->_parents)
			parents.push_back(node_ptr->_virus->_get_id());
		return parents;

	}


	Virus& operator[](id_type const & id) const {
		// node_ptr to weak_ptr do wezla reprezentujacego wirus o identyfikatorze id
		auto node_ptr = get_node(id);
		return node_ptr->_virus;
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

	// zwraca weak_ptr do wirusa o numerze id, albo gdy ten nie istnieje wyrzuca wyjatek
	auto get_node(id_type const & id) const {
		auto it = _all_nodes.find(id);
		if (it == _all_nodes.end())
			throw new VirusNotFound();
		// it to iterator wskazujacy na pare <id, std:weak_ptr<node>>
		// *(it->second) jest typu std::weak_ptr<node>
		return (it->second);
	}

};

#endif /* __VIRUS_GENEALOGY__ */