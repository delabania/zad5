#ifndef __VIRUS_GENEALOGY__
#define __VIRUS_GENEALOGY__

#include <vector>
#include <algorithm>
#include <memory>
#include <map>
#include <cassert>


class VirusNotFound : public std::exception {
    virtual const char *what() const throw() {
        return "VirusNotFound";
    }
} virus_not_found;

class VirusAlreadyCreated : public std::exception {
    virtual const char *what() const throw() {
        return "VirusAlreadyCreated";
    }
} virus_already_created;

class TriedToRemoveStemVirus : public std::exception {
    virtual const char *what() const throw() {
        return "TriedToRemoveStemVirus";
    }
} tried_to_remove_stem_virus;

template<class Virus>
class VirusGenealogy {
    using id_type = typename Virus::id_type;

public:
    VirusGenealogy(const VirusGenealogy &) = delete;

    VirusGenealogy &operator=(const VirusGenealogy &) = delete;

    /**
     * Tworzy wierzchłoek trzymający wirus macierzysty w grafie oraz umieszcza go w mapie trzymającej wierzchołki grafu
     * @param stem_id
     */
    VirusGenealogy(id_type const &stem_id) {
        try {
            _stem = std::make_shared<node>(stem_id);
            _all_nodes.emplace(stem_id, _stem);
        } catch (std::exception &e) {
            _stem.reset();
            throw;
        }
    }

    /**
     * @return id wirusa macierzystego tego grafu genealogii
     */
    id_type get_stem_id() const {
        return _stem->_virus->get_id();
    }

    /**
     * Sprawdza czy wirus o danym id istnieje w mapie trzymającej węzły grafu genealogii i czy jest aktywny
     * @param id
     * @return true jeśli wirus o podanym id istnieje w grafie genealogii i jest aktywny, false wpp
     */
    bool exists(id_type const &id) const {
        auto it = _all_nodes.find(id);
        return !(it == _all_nodes.end() || it->second.expired());
    }

    /**
     * @param id
     * @return wektor zawierający id dzieci danego węzła w grafie genealogii
     */
    std::vector<id_type> get_children(id_type const &id) const {
        auto current = get_shared_ptr_to_node(id);
        std::vector<id_type> children;

        for (auto &node_ptr : current->_children)
            children.push_back(node_ptr->_virus->get_id());
        return children;
    }

    /**
     * @param id
     * @return wektor zawierający id rodziców danego węzła w grafie genealogii
     */
    std::vector<id_type> get_parents(id_type const &id) const {
        auto current = get_shared_ptr_to_node(id);
        std::vector<id_type> parents;
        for (auto &node_ptr : current->_parents) {
            if (!node_ptr.expired()) {
                auto ptr = node_ptr.lock();
                parents.push_back(ptr->_virus->get_id());
            }
        }
        return parents;

    }

    /**
     * Overload operatora []
     * @param id
     * @return zwraca wirus o podanym id
     */
    Virus &operator[](id_type const &id) const {
        auto node_ptr = get_shared_ptr_to_node(id);
        return *(node_ptr->_virus);
    }

    /**
     * Tworzy nowy wierzchołek grafu genealogii dla wirusa o podanym id i dla rodzica o podanym parent_id
     * @param id
     * @param parent_id
     */
    void create(id_type const &id, id_type const &parent_id) {
        if (exists(id)) throw virus_already_created;
        std::shared_ptr<node> temp_ptr = make_new_node(id);

        try {
            connect(id, parent_id);
        } catch (std::exception &e) {
            throw;
        }

    }

    /**
     * Tworzy nowy wierzchołek grafu genealogii dla wirusa o podanym id i dla rodziców z podanego wektora
     * @param id
     * @param parent_ids
     */
    void create(id_type const &id, std::vector<id_type> const &parent_ids) {
        if (exists(id)) throw virus_already_created;
        if (parent_ids.size() == 0) throw virus_not_found;
        std::shared_ptr<node> temp_ptr = make_new_node(id);

        try {
            for (auto &parent_id : parent_ids) {
                connect(id, parent_id);
            }
        } catch (std::exception &e) {
            throw;
        }
    }

    /**
     * Tworzy nowe połączenie między węzłami, o ile jeszcze nie istnieje
     * Rzuca wyjątek VirusNotFound jeśli którykolwiek z nich nie istnieje
     * @param child_id
     * @param parent_id
     */
    void connect(id_type const &child_id, id_type const &parent_id) {
        if (!exists(child_id)) throw virus_not_found;
        std::shared_ptr<node> parent_node_shared_ptr = get_shared_ptr_to_node(parent_id);

        if (get_iterator_to_child(parent_id, child_id) !=
            parent_node_shared_ptr->_children.end())
            return;

        std::shared_ptr<node> child_node_shared_ptr = get_shared_ptr_to_node(child_id);
        std::weak_ptr<node> parent_node_weak_ptr = parent_node_shared_ptr;

        try {
            child_node_shared_ptr->_parents.push_back(parent_node_weak_ptr);
            parent_node_shared_ptr->_children.push_back(child_node_shared_ptr);
        } catch (std::exception &e) {
            throw;
        }
    }

    /**
     * Usuwa węzeł reprezentujący wirus wraz z jego dziećmi, o ile jest ich jedynym rodzicem
     * W pętli usuwa wskaźnik do usuwanego wierzchołka z wektorów dzieci u jego rodziców
     * @param id
     */
    void remove(id_type const &id) {
        if (_stem->_virus->get_id() == id) throw tried_to_remove_stem_virus;
        auto node_to_remove = get_shared_ptr_to_node(id);

        try {
            for (auto &parent_node_weak_ptr : node_to_remove->_parents) {
                if (!parent_node_weak_ptr.expired()) {
                    auto parent_node_shared_ptr = parent_node_weak_ptr.lock();

                    auto it = get_iterator_to_child(parent_node_shared_ptr->_virus->get_id(), id);
                    parent_node_shared_ptr->_children.erase(it);
                }
            }
        } catch (std::exception &e) {
            for (auto parent_node_weak_ptr: node_to_remove->_parents) {
                id_type parent_id = parent_node_weak_ptr.lock()->_virus->get_id();
                connect(id, parent_id);
            }
        }
    }


private:
    /**
     * Struktura przechowująca wierzchołek grafu genealogii
     * Zawiera wskaźnik do danego wirusa,
     * wektor wskaźników do jego następników.
     * wektor wskaźników do jego poprzednikow
     */
    struct node {
        std::unique_ptr<Virus> _virus;
        std::vector<std::shared_ptr<node> > _children;
        std::vector<std::weak_ptr<node> > _parents;

        node(id_type const &stem_id) : _virus(std::make_unique<Virus>(stem_id)) {}
    };

    /**
     * Mapa wszystkich potomkow wirusa macierzystego
     */
    std::map<id_type, std::weak_ptr<node> > _all_nodes;

    /**
     * Wirus macierzysty
     */
    std::shared_ptr<node> _stem;

    /**
     * Podaje wskaźnik do wirusa o danym id
     * @param id
     * @return shared_ptr do wirusa o numerze id, a gdy ten nie istnieje wyrzuca wyjatek
     */
    std::shared_ptr<node> get_shared_ptr_to_node(id_type const &id) const {
        if (!exists(id)) throw virus_not_found;
        auto it = _all_nodes.find(id);
        auto ptr = it->second.lock();
        return ptr;
    }

    /**
     * Zwraca iterator do wierzcholka w wektorze synow, ktory reprezentuje wirus o identyfikatorze child_id
     * @param parent
     * @param child_id
     * @return iterator do elementu w mapie trzymającego wirus o id równym child_id
     */
    //TODO czy ma rzucać wyjątiem w przypadku gdy ==.end()? - wtedy inaczej działa connect
    auto get_iterator_to_child(id_type const &parent_id, id_type const &child_id) {
        auto parent = get_shared_ptr_to_node(parent_id);
        auto it = std::find_if(parent->_children.begin(), parent->_children.end(),
                               [&child_id](std::shared_ptr<node> child) {
                                   return child->_virus->get_id() == child_id;
                               });
        //if (it == parent->_children.end()) throw virus_not_found;
        return it;
    }

    /**
     * Tworzy nowy węzeł w grafie genealogii i dodaje go do mapy
     * @param id
     * @return wskaźnik do nowego węzła w mapie
     */
    std::shared_ptr<node> make_new_node(id_type const &id) {
        std::shared_ptr<node> new_node;

        try {
            new_node = std::make_shared<node>(id);
            _all_nodes.emplace(id, new_node);
            return new_node;
        } catch (std::exception &e) {
            new_node.reset();
            throw;
        }
    }

};

#endif /* __VIRUS_GENEALOGY__ */