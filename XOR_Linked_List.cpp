// XOR.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <iostream>

using ptr = uintptr_t;

struct node {
    int val;
    ptr npx;
};

void LOG(int n) {
    if (n == NULL) {
        std::cout <<"NULL\n";
    }
    else {
        std::cout << n << "\n";
    }
}

class XOR {

private:

    node* head_beg, * head_end;
    node* prev, *next;
    node* actual;
    
    void reset_actual()
    {
        prev = nullptr;
        actual = nullptr;
        next = nullptr;
    }

    

    void init_actual(node* new_el)//uruchamiany gdy dodajemy pierwszy element do listy
    {
        prev = head_beg;
        actual = new_el;
        next = head_end;
    }
public:
    XOR() :head_beg(new node()), head_end(new node()), prev(nullptr),next(nullptr), actual(nullptr) {
        head_beg->npx = (ptr)head_end;
        head_beg->val = NULL;
        head_end->npx = (ptr)head_beg;
        head_end->val = NULL;
    }
    
    ~XOR() {
        node* prevs = (node*)head_beg;
        node* current = (node*)head_beg->npx;

        while (current->val != NEGATIVE) {
            node* temp = current;
            current = (node*)((ptr)prevs ^ current->npx);
            free(prevs);
            prevs = temp;
        }
        if(prevs != NULL)
            free(prevs);
        free(current);
    }
    
    void add_act_n(int n) {
        if (actual != nullptr) { //sprawdza czy aktualny istnieje

            node* new_el = new node();
            new_el->val = n;
            new_el->npx = (ptr)prev ^ (ptr)actual;

            actual->npx = (ptr)new_el ^ (ptr)next;


            if (prev == head_beg) { //początek tablicy
                prev->npx = (ptr)new_el;
            }
            else {//środek tablicy             //poprzednik poprzednika
                prev->npx = (ptr)((node*)(prev->npx ^ (ptr)actual)) ^ (ptr)new_el;
            }
            prev = new_el;
        }
    }

    void add_beg_n(int n) {
        

        node* new_el = new node();
        new_el->val = n;
        new_el->npx = (ptr)head_beg ^ head_beg->npx; //npx = początek ^ pierwszy element


        if (((node*)head_beg->npx)->val == NULL) { //czy lista jest pusta
            ((node*)head_beg->npx)->npx = (ptr)new_el; //koniec niech wskazuje na pierwszy element listy
            init_actual(new_el);
        }
        else {                            //nowy element      //element drugi listy
            ((node*)head_beg->npx)->npx = (ptr)new_el ^ ((ptr)head_beg ^ ((node*)head_beg->npx)->npx);
            if (!prev->val && !next->val)
                prev = new_el;
            else if (prev == head_beg) //dodajemy kolejny element
                prev = new_el;
        }

        head_beg->npx = (ptr)new_el;

    }

    void add_end_n(int n) {


        node* new_el = new node();
        new_el->val = n;
        new_el->npx = (ptr)head_end ^ head_end->npx; //npx = koniec ^ ostatni element



        if (((node*)head_end->npx)->val == NULL) { //czy lista jest pusta
            ((node*)head_end->npx)->npx = (ptr)new_el; //początek niech wskazuje na ostatni element listy
            init_actual(new_el);
        }
        else {                            //nowy element      //element przedostatni listy
            ((node*)head_end->npx)->npx = (ptr)new_el ^ ((ptr)head_end ^ ((node*)head_end->npx)->npx);

            if (!next->val && !prev->val)//dodajemy drugi element do listy
                next = new_el;

            else if (next == head_end) //dodajemy kolejny element
                next = new_el;
        }

        head_end->npx = (ptr)new_el;

    }

    int get_actual() const {
        if (actual != nullptr)
            return actual->val;
        else
            return NULL;
    }

    int get_next() {

        if (actual != nullptr && next) {
            if (next->val == NULL) { //koniec listy, więc zawijamy
                prev = head_beg;
                actual = (node*)head_beg->npx;
                if (actual->val == NULL) { // jeśli prawda tzn. że opróżniliśmy listę
                    reset_actual();
                }else
                    next = (node*)((ptr)head_beg ^ actual->npx);
            }
            else {
                node* temp = next;
                next = (node*)((ptr)actual ^ next->npx);
                prev = actual;
                actual = temp;
            }
            return get_actual();
        }
        else {
            return NULL;
        }
    }

    int get_prev() {
        if (actual != nullptr) {
            if (prev->val == NULL) {
                next = head_end;
                actual = (node*)head_end->npx;
                if (actual->val == NULL) { // jeśli prawda tzn. że opróżniliśmy listę
                    reset_actual();
                }
                else
                    prev = (node*)((ptr)head_end ^ actual->npx);
            }
            else {
                node* temp = prev;
                prev = (node*)((ptr)actual ^ prev->npx);
                next = actual;
                actual = temp;
            }
            return get_actual();
        }
        else {
            return NULL;
        }
    }
        
    void del_beg() {
        if (((node*)head_beg->npx)->val != NULL) { //sprawdza czy element pierwszy nie jest końcem

            node* first_temp = (node*)head_beg->npx; //trzymamy wskaźnik do pierwszego elementu, aby po wszystkich zmianach go usunąć



            ptr new_first_address = (ptr)head_beg ^ ((node*)head_beg->npx)->npx; //head->npx = adres drugiego elementu

            if (first_temp->npx == ((ptr)head_beg ^ (ptr)head_end)) { // tylko jeden element listy
                head_beg->npx = (ptr)head_end;
                head_end->npx = (ptr)head_beg;
                reset_actual();
            }
            //else if (((node*)new_first_address)->val == NULL) { //sprawdza czy lista będzie pusta
            //    head_end->npx = (ptr)head_beg; //ustawiamy end->npx na head_beg
            //}
            else { //w liscie zostanie chociaz jeden  element

                ((node*)new_first_address)->npx = (ptr)head_beg ^ ((ptr)first_temp ^ ((node*)new_first_address)->npx); //ustawiam npx drugiego elementu
                head_beg->npx = new_first_address; //drugi element jest teraz pierwszy

            }

            //usuwając pierwszy element może on być actual lub prev, ale na pewno nie będzie next
            if (first_temp == actual) { //jeśli usuwany element jest actual 
                next = head_end;
                actual = (node*)head_end->npx;
                prev = (node*)((ptr)head_end ^ actual->npx);
            }
            else if (first_temp == prev) { //jeśli usuwany element jest prev 
                prev = (node*)head_beg->npx;
            }

            free(first_temp); //zwalniamy pamięć z usuniętego elementu

        }
    }

    void del_end() {
        if (((node*)head_end->npx)->val != NULL) { //sprawdza czy element ostatni nie jest początkiem

            node* last_temp = (node*)head_end->npx; //trzymamy wskaźnik do ostatniego elementu, aby po wszystkich zmianach go usunąć



            ptr new_last_address = (ptr)head_end ^ ((node*)head_end->npx)->npx; //head->npx = adres drugiego elementu

            if (last_temp->npx == ((ptr)head_beg ^ (ptr)head_end)) { // tylko jeden element listy
                head_beg->npx = (ptr)head_end;
                head_end->npx = (ptr)head_beg;
                reset_actual();
            }
            //else if (((node*)new_last_address)->val == NULL) { //sprawdza czy lista będzie pusta
            //    head_beg->npx = (ptr)head_end; //ustawiamy beg->npx na head_end
            //}
            else { //w liscie zostanie chociaz jeden  element

                ((node*)new_last_address)->npx = (ptr)head_end ^ ((ptr)last_temp ^ ((node*)new_last_address)->npx); //ustawiam npx przedostatniego elementu
                head_end->npx = new_last_address; //przedostatni element jest teraz ostatni

            }

            //usuwając ostatni element może on być actual lub next, ale na pewno nie będzie prev
            if (last_temp == actual) { //jeśli usuwany element jest actual 
                next = head_end;
                actual = (node*)head_beg->npx;
                prev = (node*)((ptr)head_end ^ actual->npx);
            }
            else if (last_temp == next) { //jeśli usuwany element jest next 
                next = head_end;
            }

            free(last_temp); //zwalniamy pamięć z usuniętego elementu

        }
    }

    void del_act() {
        if (prev == head_beg && next == head_end) { //actual jest jedynym elementem
            head_beg->npx = (ptr)head_end;
            head_end->npx = (ptr)head_beg;
            free(actual);
            reset_actual();
        }
        else if (prev == head_beg) { //actual jest pierwszym elementem
            del_beg();
        }
        else if (next == head_end) { //actual jest ostatnim elementem
            del_end();
        }
        else if(actual!=nullptr) { //actual jest w środku
            node* temp_actual = actual;

            actual = prev;
            prev = (node*)(prev->npx ^ (ptr)temp_actual);
            actual->npx = (ptr)next ^ (ptr)prev;
                         //next nexta
            next->npx = (next->npx ^ (ptr)temp_actual) ^ (ptr)actual;
            //nowy element to początek lsity
            if (prev == head_beg) {
                prev->npx = (ptr)actual;
            }

            //nowy element to koniec listy
            if (next == head_end) {
                next->npx = (ptr)actual;
            }

            free(temp_actual);
        }
    }

    void del_val(int n) {
        node* prevs = (node*)head_beg;
        node* current = (node*)head_beg->npx;

        if (current->val == NULL)
            LOG(NULL);

        while (current->val != NULL) {
            
            //usuwanie elementu z tą wartością
            if (current->val == n) {
                if (current == (node*)head_beg->npx) { //pierwszy element listy
                    node* nexts = (node*)(current->npx ^ (ptr)prevs);
                    del_beg();
                    current = nexts;
                }
                else if (current == (node*)head_end->npx) { //ostatni element listy
                    node* nexts = (node*)(current->npx ^ (ptr)prevs);
                    del_end();

                    current = nexts;
                }
                else { // w środku listy

                    if (current == actual) { //aktualny element listy
                        node* nexts = (node*)(current->npx ^ (ptr)prevs);
                        del_act();
                        current = nexts;
                    }
                    else { //inny niż aktualny
                        node* temp = current; //wskaźnik na usuwany element

                        
                        node* del_next = (node*)((ptr)prevs ^ current->npx); //nastepca usuwanego
                        node* del_prev = prevs; //poprzednik usuwanego

                                        //następca następcy               //poprzednik usuwanego
                        del_next->npx = ((del_next->npx ^ (ptr)current) ^ (ptr)del_prev);
                                        
                                        //poprzednik porzednika           //poprzednik usuwanego
                        del_prev->npx = ((del_prev->npx ^ (ptr)current) ^ (ptr)del_next);


                        //wymaga zaktualizować prev lub next
                        if (current == prev) {
                            prev = del_prev;
                        }
                        else if (current == next) {
                            next = del_next;
                        }

                        current = del_next;
                        prevs = del_prev;
                        free(temp);
                    }
                }
            }
            else {
                //dalsza iteracja
                node* temp = current;
                current = (node*)((ptr)prevs ^ current->npx);
                prevs = temp;
            }
        }
        std::cout << "\n";
    }

    void print_forward() const {
        node* prevs = (node*)head_beg;
        node* pointer = (node*)head_beg->npx;
        if (pointer->val == NULL)
            LOG(NULL);
        while (pointer->val != NULL) {
            std::cout << pointer->val <<" ";
            node* temp = pointer;
            pointer = (node*)((ptr)prevs ^ pointer->npx);
            prevs = temp;
        }
        std::cout << "\n";
    }

    void print_backward() const {
        node* nexts = (node*)head_end;
        node* pointer = (node*)head_end->npx;
        if (pointer->val == NULL)
            LOG(NULL);
        while (pointer->val != NULL) {
            std::cout << pointer->val << " ";
            node* temp = pointer;
            pointer = (node*)((ptr)nexts ^ pointer->npx);
            nexts = temp;
        }
        std::cout << "\n";
    }
};


int main()
{
    XOR list;
    char* buffer = new char[20];
    int n_buffer;

    while (std::cin >> buffer) {

        if (!strcmp(buffer, "ADD_BEG")) {
            std::cin >> n_buffer;
            list.add_beg_n(n_buffer);
        }
        else if (!strcmp(buffer, "ADD_END")) {
            std::cin >> n_buffer;
            list.add_end_n(n_buffer);
        }
        else if (!strcmp(buffer, "ADD_ACT")) {
            std::cin >> n_buffer;
            list.add_act_n(n_buffer);
        }
        else if (!strcmp(buffer, "ACTUAL")) {
            LOG(list.get_actual());
        }
        else if (!strcmp(buffer, "NEXT")) {
            LOG(list.get_next());
        }
        else if (!strcmp(buffer, "PREV")) {
            LOG(list.get_prev());
        }
        else if (!strcmp(buffer, "DEL_BEG")) {
            list.del_beg();
        }
        else if (!strcmp(buffer, "DEL_END")) {
            list.del_end();
        }
        else if (!strcmp(buffer, "DEL_ACT")) {
            list.del_act();
        }
        else if (!strcmp(buffer, "DEL_VAL")) {
            std::cin >> n_buffer;
            list.del_val(n_buffer);
        }
        else if (!strcmp(buffer, "PRINT_FORWARD")) {
            list.print_forward();
        }
        else if (!strcmp(buffer, "PRINT_BACKWARD")) {
            list.print_backward();
        }
    }

    delete[] buffer;

}
