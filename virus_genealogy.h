#ifndef __VIRUS_GENEALOGY__
#define __VIRUS_GENEALOGY__

#include <vector>
#include <algorithm>
#include <memory>
#include <map>
#include <cassert>

#include <iostream>


// Pomocniczo, zeby sprawdzic czy sie kompiluje
// Do usuniecia po skonczeniu

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
} virus_not_found;

class VirusAlreadyCreated : public std::exception {
	virtual const char* what() const throw() {
		return "VirusAlreadyCreated";
	}
} virus_already_created;

class TriedToRemoveStemVirus : public std::exception {
	virtual const char* what() const throw() {
		return "TriedToRemoveStemVirus";
	}
} tried_to_remove_stem_virus;


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
		return _stem->_virus->get_id();
	}


	bool exists(id_type const &id) const noexcept {
		auto it = _all_nodes.find(id);
		// jezeli nie wezla w mapie, albo jest, ale wczesniej zostal usuniety 
		if(it == _all_nodes.end() || it->second.expired())
			return false;
		// jest w mapie i jest nadal aktywny
		return true;
	}


	//@TODO : straszny copypaste z get_parents! Wydzielic do jednej funkcji
	std::vector<id_type> get_children(id_type const &id) const {
		// wyszukaj wezel o numerze id, gdy nie istnieje rzuc wyjatek => wewnatrz funkcji get_node_shared_ptr
		auto current = get_node_shared_ptr(id);
		std::vector<id_type> children;
		// current->_children to wektor <shared_ptr> nastepnikow (dzieci) wezla current
		for (auto & node_ptr : current->_children)
			children.push_back(node_ptr->_virus->get_id());
		return children;
	}


	std::vector<id_type> get_parents(id_type const &id) const {
		// analogicznie jak w get_children
		auto current = get_node_shared_ptr(id);
		std::vector<id_type> parents;
		// current->_parent to wektor <weak_ptr> nastepnikow (dzieci) wezla current
		// przed uzyciem
		for (auto & node_ptr : current->_parents) {
			if (!node_ptr.expired()) {
				auto ptr = node_ptr.lock();
				parents.push_back(ptr->_virus->get_id());
			}
		}

		return parents;

	}


	Virus& operator[](id_type const & id) const {
		// node_ptr to shared_ptr do wezla reprezentujacego wirus o identyfikatorze id
		auto node_ptr = get_node_shared_ptr(id);
		return *(node_ptr->_virus);
	}

	void create(id_type const & id, id_type const & parent_id) {
		auto it = _all_nodes.find(id);
		if (it != _all_nodes.end() && !it->second.expired())
			throw virus_already_created;
		auto parent_node_shared_ptr = get_node_shared_ptr(parent_id);
		// wszystko jest ok => stworz nowy wezel
		auto node_shared_ptr = std::make_shared<node>(id);
		// wstaw do mapy aktualny wierzcholek
		_all_nodes[id] = node_shared_ptr;

		// stworz pomocniczy slaby wskaznik do ojca
		std::weak_ptr<node> parent_weak_ptr = parent_node_shared_ptr;

		// dodaj nowe dziecko do ojca
		parent_node_shared_ptr->_children.push_back(node_shared_ptr);
		// uzyj slabego wskaznika na ojca zeby wstawic go do listy ojcow aktualnego wezla
		node_shared_ptr->_parents.push_back(parent_weak_ptr);
	}

	//@TODO : znowu copypaste!
	void create(id_type const & id, std::vector<id_type> const & parent_ids) {
		auto it = _all_nodes.find(id);
		if (it != _all_nodes.end() && !it->second.expired())
			throw virus_already_created;
		if (parent_ids.size() == 0)
			throw virus_not_found;
		// wektor wskaznikow do potencjalnych ojcow
		std::vector <std::shared_ptr<node> > parent_nodes;
		// dla kazdego id wstaw do wektora weak_ptr do danego wezla
		for (auto & parent_id : parent_ids) {
			parent_nodes.push_back(get_node_shared_ptr(parent_id));
		}

		//dalej analogicznie jak w metodzie create z jednym ojcem
		auto node_ptr = std::make_shared<node>(id);
		// wstaw do mapy aktualny wierzcholek
		_all_nodes[id] = node_ptr;


		for (auto & parent_node_ptr : parent_nodes) {
			std::weak_ptr<node> parent_weak_ptr = parent_node_ptr;
			parent_node_ptr->_children.push_back(node_ptr);
			node_ptr->_parents.push_back(parent_weak_ptr);
		}
	}


	//@TODO : jezeli krawedz istnieje
	void connect(id_type const & child_id, id_type const & parent_id) {
		auto parent_node_shared_ptr = get_node_shared_ptr(parent_id);
		// Jesli krawedz istnieje to nic nie rob
		if (get_iterator_to_child_ptr(parent_node_shared_ptr, child_id) !=
		        parent_node_shared_ptr->_children.end())
			return;
		// Wpp stworz polaczenie miedzy ojcem a dzieckiem
		auto parent_node_weak_ptr = parent_node_shared_ptr;
		auto child_node_shared_ptr = get_node_shared_ptr(child_id);

		child_node_shared_ptr->_parents.push_back(parent_node_weak_ptr);
		parent_node_shared_ptr->_children.push_back(child_node_shared_ptr);
	}


	void remove(id_type const & id) {
		if (_stem->_virus->get_id() == id)
			throw tried_to_remove_stem_virus;
		auto node_to_remove = get_node_shared_ptr(id);
		assert(node_to_remove->_virus->get_id() == id);

		// usun wskaznik do aktualnego wierzcholka ze wszystkich jego rodzicow
		for (auto & parent_node_weak_ptr : node_to_remove->_parents) {
			if (!parent_node_weak_ptr.expired()) {
				auto parent_node_shared_ptr = parent_node_weak_ptr.lock();
				delete_ptr_to_child(parent_node_shared_ptr, id);
			}
		}
		// reszte robi za nas (domyslny) destruktor node() ktory zostanie wywolany zaraz
		// po opuszczeniu funkcji -> wyczysci wektory dzieci i rodzica, a takze usunie wirusa
		// a wskaznik w mapie bedzie `expired`
	}


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

	// zwraca shared_ptr do wirusa o numerze id, albo gdy ten nie istnieje wyrzuca wyjatek
	std::shared_ptr<node> get_node_shared_ptr(id_type const & id) const {
		auto it = _all_nodes.find(id);
		if (it == _all_nodes.end() || it->second.expired())
			throw virus_not_found;
		// it to iterator wskazujacy na pare <id, std:weak_ptr<node>>
		// (*it)->second jest typu std::weak_ptr<node>
		auto ptr = it->second.lock();
		return ptr;
	}

    //Nie wiem czy jest najlepszym pomysłem wrzucanie tu shared_pointer w parametrze - chyba byłoby lepiej przekazać const & parent
	auto get_iterator_to_child_ptr(std::shared_ptr<node> parent, id_type const & child_id) {
		auto it = std::find_if (parent->_children.begin(), parent->_children.end(),
		[&child_id](std::shared_ptr<node> child) {
			return child->_virus->get_id() == child_id;
		});
		return it;
	}

	// zwraca iterator do wierzcholka w wektorze synow, ktory reprezentuje
	// wirus o identyfikatorze child_id
	void delete_ptr_to_child(std::shared_ptr<node> parent, id_type const & child_id) {
		//usun dziecko
		auto it = get_iterator_to_child_ptr(parent, child_id);
		assert(it != parent->_children.end());
		parent->_children.erase(it);
	}

};

#endif /* __VIRUS_GENEALOGY__ */