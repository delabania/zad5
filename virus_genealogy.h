#ifndef __VIRUS_GENEALOGY__
#define __VIRUS_GENEALOGY__

#include <vector>


template <class Virus>
class VirusGenealogy {
	using id_type = typename Virus::id_type;
	
public:
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

};


#endif /* __VIRUS_GENEALOGY__ */