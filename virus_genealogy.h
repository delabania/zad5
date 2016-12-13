#ifndef __VIRUS_GENEALOGY__
#define __VIRUS_GENEALOGY__

#include <vector>
#include <memory>

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
template <class Virus>
class VirusGenealogy {
	using id_type = typename Virus::id_type;

public:
	VirusGenealogy(const VirusGenealogy &) = delete;
	VirusGenealogy & operator=(const VirusGenealogy &) = delete;

	VirusGenealogy(id_type const &stem_id);
	id_type get_stem_id() const;
	std::vector<id_type> get_children(id_type const &id) const;
	std::vector<id_type> get_parents(id_type const &id) const;
	bool exists(id_type const &id) const;
	Virus& operator[](id_type const &id) const;
	void create(id_type const &id, id_type const &parent_id);
	void create(id_type const &id, std::vector<id_type> const &parent_ids);
	void connect(id_type const &child_id, id_type const &parent_id);
	void remove(id_type const &id);

private:
	struct node {
		//http://stackoverflow.com/questions/27348396/smart-pointers-for-graph-representation-vertex-neighbors-in-c11
		std::vector<std::shared_ptr<node> > children;
		std::vector<std::weak_ptr<node> > parents;
		//virus_unique_ptr virus;
	};


};

#endif /* __VIRUS_GENEALOGY__ */