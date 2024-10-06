#include <iostream>
#include <string>

class User {
private:
    std::string _username;
    std::string raw_password;
    unsigned long posts;
public:
    User(std::string lusername = "unset", std::string lpass = "unset") {
        _username = lusername;
        raw_password = lpass;

        posts = 0;
     
    }

    void inc_posts() {
        posts++;
    }
    std::string* username() {
        return &_username;
    }
    std::string password() {
        return raw_password;
    }

    ~User() {
       /* delete _username.data();
        delete raw_password.data();*/
    }
};


/* I understand I could write this with vector, but it's a bit more fun. */
class Database {
private:
    User* user_database = nullptr;
    long* uuid_map = nullptr;

    long next_uuid = 1000;
    int size = 0, res_size = 0;
    int deleted_users = 0;
public:
    Database() {
        user_database = new User[5];
        uuid_map = new long[5];
        res_size = 5;
    }

    long insert_user(User n) { 
        if (res_size > size) {
            user_database[size] = n;
            uuid_map[size] = next_uuid;

            next_uuid++;
            size++;

            return (next_uuid-1);
        }
        
        User* ndb = new User[size + 6];
        memcpy(ndb, user_database, (sizeof(User) * size));
        ndb[size] = n;
        
        long* nuidmap = new long[size + 6];
        memcpy(nuidmap, uuid_map, sizeof(long) * size);
        nuidmap[size] = next_uuid;

        delete[] user_database;
        user_database = ndb;

        free(uuid_map);
        uuid_map = nuidmap;
        
        next_uuid++;
        size++;
        res_size += 6;
        
        return (next_uuid - 1);
    }

    User* lookup(long uuid) {
        long est_loc = (uuid - 1000 - deleted_users) - 1; //minus base.
        if (est_loc < 0 || est_loc > size) { est_loc = 0; }

        for (int i = est_loc; i < size; i++) {
            if (uuid_map[i] == uuid) {
                return &user_database[i];
            }
        }
       // printf("Uuid not found!\n");

        return new User("0","0");
    }
    
    void delete_uuid(long uuid) {
        User* new_udb = new User[size - 1];
        long* new_uuids = new long[size - 1];

        size_t loc = 0;
        for (int i = 0; i < size; i++) {
            if (uuid_map[i] == uuid) {
                loc = i;
                break;
            }
        }

        std::copy(user_database, user_database + loc, new_udb);
        std::copy(user_database + loc + 1, user_database + size, new_udb + loc);

        std::copy(uuid_map, uuid_map + loc, new_uuids);
        std::copy(uuid_map + loc + 1, uuid_map + size, new_uuids + loc);

        
        delete[] user_database;
        delete[] uuid_map;

        user_database = new_udb;
        uuid_map = new_uuids;

        size--;
    }



    void change_user_name(long uuid, std::string new_name) {
        *(lookup(uuid)->username()) = new_name;
    }
    void display_users() {
        for (int i = 0; i < size; i++) {
            std::cout << uuid_map[i] << " -> " << *user_database[i].username() << " | " << user_database[i].password() << "\n";
        }
    }

};

namespace Posts { //Idk, felt like making this a struct instead of a class.

    typedef struct Post {
        char* title;
        char* text;
        long author_uuid; //cant be ptr to author because access rights.s

        int reply_count;
        Post* replies; //We won't be doing reserve memory for every post reply- way too wasteful. Only useful for DB since there (should be) only one.
    };

    Post* new_post(std::string title, std::string text, long author) {
        Post* np = new Post;

        np->title = (char*)malloc(sizeof(char) * title.size());
        strcpy(np->title, title.c_str());

        np->text = (char*)malloc(sizeof(char) * text.size());
        strcpy(np->text, text.c_str());

        np->author_uuid = author;;

        np->reply_count = 0;
        return np;
    }
}


using Posts::Post; //I hate using "using" fr. First time in like 2 years
class Forum {
private:
    Database user_db;
    
    int post_count = 0;
    Post** posts;

public:

    Forum() {
        
    }

    void make_post(long uuid, std::string pass, std::string title, std::string text) {
        User* user = user_db.lookup(uuid);

        if (user->password() != pass) {
            printf("Auth failed!\n");
            return;
        }

        Post* np = Posts::new_post(title, text, uuid);
        posts = (Post**)realloc(posts, (sizeof(Post*) * (post_count + 1)));
        posts[post_count] = np;
        post_count++;

        user->inc_posts();
    }

    unsigned long signup(std::string username, std::string pass) { //Return UUID for fun!
        return user_db.insert_user(User(username, pass));
    }

    void display_all_posts() {
        if (!post_count) { return; }
     for(int n = 0; n < post_count; n++) {
            Post* p_ptr = posts[n];
            auto author = user_db.lookup(p_ptr->author_uuid);

            std::cout << p_ptr->title << " || By " << *author->username() << "\n"
                << p_ptr->text << "\n(" << p_ptr->reply_count << " Replies)\n";

            p_ptr++;
        }
    }

};

