#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

// ─────────────────────────────────────────────
// Constants
// ─────────────────────────────────────────────
const int MAX_USERS = 30;
const int MAX_PAGES = 20;
const int MAX_POSTS = 30;
const int MAX_COMMENTS = 20;
const int MAX_POST_COMMENTS = 10;
const int MAX_LIKES = 10;
const int MAX_FRIENDS = 20;
const int MAX_LIKED_PAGES = 20;
const int MAX_PAGE_POSTS = 20;
const int STR_LEN = 256;

// ─────────────────────────────────────────────
// Helper Class (Replaces Global Functions)
// ─────────────────────────────────────────────
class Helper {
public:
    static void safeStrCopy(char* dest, const char* src, int maxLen = STR_LEN) {
        if (!src) { dest[0] = '\0'; return; }
        int i = 0;
        while (i < maxLen - 1 && src[i] != '\0') { dest[i] = src[i]; i++; }
        dest[i] = '\0';
    }

    static bool strEq(const char* a, const char* b) {
        if (!a || !b) return false;
        int i = 0;
        while (a[i] != '\0' && b[i] != '\0') {
            if (a[i] != b[i]) return false;
            i++;
        }
        return a[i] == '\0' && b[i] == '\0';
    }

    static void trim(char* s) {
        if (!s) return;
        int start = 0;
        while (s[start] == ' ' || s[start] == '\t') start++;
        int len = 0;
        while (s[start + len] != '\0') len++;
        for (int i = 0; i < len; i++) s[i] = s[start + i];
        s[len] = '\0';
        int end = len - 1;
        while (end >= 0 && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r' || s[end] == '\n'))
            s[end--] = '\0';
    }
};

// ─────────────────────────────────────────────
// Forward declarations
// ─────────────────────────────────────────────
class User;
class Page;
class Post;
class Comment;

// ─────────────────────────────────────────────
// Date
// ─────────────────────────────────────────────
struct Date {
    int day, month, year;
    Date() : day(0), month(0), year(0) {}
    Date(int d, int m, int y) : day(d), month(m), year(y) {}

    bool isWithin24Hours(const Date& current) const {
        if (year == current.year && month == current.month) {
            return (current.day - day) <= 1 && (current.day - day) >= 0;
        }
        return false;
    }
};

// ─────────────────────────────────────────────
// Activity
// ─────────────────────────────────────────────
class Activity {
public:
    int    type;   // 1=feeling, 2=thinking about, 3=making, 4=celebrating
    char   value[STR_LEN];

    Activity() : type(0) { value[0] = '\0'; }

    void set(int t, const char* v) {
        type = t;
        Helper::safeStrCopy(value, v);
    }

    void print() const {
        switch (type) {
        case 1: cout << "is feeling " << value; break;
        case 2: cout << "is thinking about " << value; break;
        case 3: cout << "is making " << value; break;
        case 4: cout << "is Celebrating " << value; break;
        default: break;
        }
    }
};

// ─────────────────────────────────────────────
// Comment
// ─────────────────────────────────────────────
class Comment {
public:
    char  id[20];
    char  postId[20];
    char  authorId[20];
    char  text[STR_LEN];

    Comment() {
        id[0] = '\0'; postId[0] = '\0';
        authorId[0] = '\0'; text[0] = '\0';
    }

    void set(const char* cid, const char* pid, const char* aid, const char* txt) {
        Helper::safeStrCopy(id, cid);
        Helper::safeStrCopy(postId, pid);
        Helper::safeStrCopy(authorId, aid);
        Helper::safeStrCopy(text, txt);
    }
};

// ─────────────────────────────────────────────
// Post
// ─────────────────────────────────────────────
class Post {
public:
    char     id[20];
    Date     date;
    char     text[STR_LEN];
    int      type;
    Activity activity;
    char     authorId[20];
    bool     isMemory;
    char     originalPostId[20];
    char     memoryText[STR_LEN];

    char** likedBy;
    int      likeCount;

    Comment** comments;
    int       commentCount;

    Post() : type(1), isMemory(false), likeCount(0), commentCount(0), likedBy(nullptr), comments(nullptr) {
        id[0] = '\0'; text[0] = '\0'; authorId[0] = '\0';
        originalPostId[0] = '\0'; memoryText[0] = '\0';
    }

    ~Post() {
        if (likedBy) {
            for (int i = 0; i < likeCount; i++) {
                if (likedBy[i]) { delete[] likedBy[i]; likedBy[i] = nullptr; }
            }
            delete[] likedBy;
        }
        if (comments) {
            delete[] comments;
        }
    }

    bool addLike(const char* uid) {
        if (likeCount >= MAX_LIKES) return false;
        if (!likedBy) {
            likedBy = new char* [MAX_LIKES];
            for (int i = 0; i < MAX_LIKES; i++) likedBy[i] = nullptr;
        }
        for (int i = 0; i < likeCount; i++)
            if (Helper::strEq(likedBy[i], uid)) return false;

        likedBy[likeCount] = new char[20];
        Helper::safeStrCopy(likedBy[likeCount], uid, 20);
        likeCount++;
        return true;
    }

    bool addComment(Comment* c) {
        if (commentCount >= MAX_POST_COMMENTS) return false;
        if (!comments) {
            comments = new Comment * [MAX_POST_COMMENTS];
            for (int i = 0; i < MAX_POST_COMMENTS; i++) comments[i] = nullptr;
        }
        comments[commentCount++] = c;
        return true;
    }
};

// ─────────────────────────────────────────────
// Page
// ─────────────────────────────────────────────
class Page {
public:
    char  id[20];
    char  title[STR_LEN];
    Post** posts;
    int    postCount;

    Page() : postCount(0), posts(nullptr) {
        id[0] = '\0'; title[0] = '\0';
    }

    ~Page() {
        if (posts) { delete[] posts; posts = nullptr; }
    }

    void addPost(Post* p) {
        if (!posts) {
            posts = new Post * [MAX_PAGE_POSTS];
            for (int i = 0; i < MAX_PAGE_POSTS; i++) posts[i] = nullptr;
        }
        if (postCount < MAX_PAGE_POSTS) posts[postCount++] = p;
    }
};

// ─────────────────────────────────────────────
// User
// ─────────────────────────────────────────────
class User {
public:
    char  id[20];
    char  name[STR_LEN];

    char** friendIds;
    int    friendCount;

    char** likedPageIds;
    int    likedPageCount;

    Post** posts;
    int    postCount;

    User() : friendCount(0), likedPageCount(0), postCount(0), friendIds(nullptr), likedPageIds(nullptr), posts(nullptr) {
        id[0] = '\0'; name[0] = '\0';
    }

    ~User() {
        if (friendIds) {
            for (int i = 0; i < friendCount; i++)
                if (friendIds[i]) delete[] friendIds[i];
            delete[] friendIds;
        }
        if (likedPageIds) {
            for (int i = 0; i < likedPageCount; i++)
                if (likedPageIds[i]) delete[] likedPageIds[i];
            delete[] likedPageIds;
        }
        if (posts) delete[] posts;
    }

    void addFriend(const char* fid) {
        if (friendCount >= MAX_FRIENDS) return;
        if (!friendIds) {
            friendIds = new char* [MAX_FRIENDS];
            for (int i = 0; i < MAX_FRIENDS; i++) friendIds[i] = nullptr;
        }
        friendIds[friendCount] = new char[20];
        Helper::safeStrCopy(friendIds[friendCount], fid, 20);
        friendCount++;
    }

    void addLikedPage(const char* pid) {
        if (likedPageCount >= MAX_LIKED_PAGES) return;
        if (!likedPageIds) {
            likedPageIds = new char* [MAX_LIKED_PAGES];
            for (int i = 0; i < MAX_LIKED_PAGES; i++) likedPageIds[i] = nullptr;
        }
        likedPageIds[likedPageCount] = new char[20];
        Helper::safeStrCopy(likedPageIds[likedPageCount], pid, 20);
        likedPageCount++;
    }

    void addPost(Post* p) {
        if (!posts) {
            posts = new Post * [MAX_PAGE_POSTS];
            for (int i = 0; i < MAX_PAGE_POSTS; i++) posts[i] = nullptr;
        }
        if (postCount < MAX_PAGE_POSTS) posts[postCount++] = p;
    }
};

// ─────────────────────────────────────────────
// SocialNetworkApp
// ─────────────────────────────────────────────
class SocialNetworkApp {
private:
    User** users;
    int       userCount;

    Page** pages;
    int       pageCount;

    Post** posts;
    int       postCount;

    Comment** comments;
    int       commentCount;

    User* currentUser;
    Date      systemDate;

    // ── Helpers ──────────────────────────────

    User* findUser(const char* id) {
        for (int i = 0; i < userCount; i++)
            if (Helper::strEq(users[i]->id, id)) return users[i];
        return nullptr;
    }

    Page* findPage(const char* id) {
        for (int i = 0; i < pageCount; i++)
            if (Helper::strEq(pages[i]->id, id)) return pages[i];
        return nullptr;
    }

    Post* findPost(const char* id) {
        for (int i = 0; i < postCount; i++)
            if (Helper::strEq(posts[i]->id, id)) return posts[i];
        return nullptr;
    }

    const char* getAuthorName(const char* id) {
        if (id[0] == 'u') {
            User* u = findUser(id);
            return u ? u->name : id;
        }
        else {
            Page* p = findPage(id);
            return p ? p->title : id;
        }
    }

    void printPost(Post* p) {
        if (!p) return;

        if (p->isMemory) {
            cout << "~~~ " << getAuthorName(p->authorId) << " shared a memory ~~~ ...("
                << p->date.day << "/" << p->date.month << "/" << p->date.year << ")" << endl;
            cout << "\"" << p->memoryText << "\"" << endl;

            Post* orig = findPost(p->originalPostId);
            if (orig) {
                int yearsAgo = systemDate.year - orig->date.year;
                cout << "(" << yearsAgo << " Year" << (yearsAgo > 1 ? "s" : "") << " Ago)" << endl;
                printPost(orig);
            }
            return;
        }

        cout << "--- ";
        const char* authorName = getAuthorName(p->authorId);
        if (p->type == 2) {
            cout << authorName << " ";
            p->activity.print();
        }
        else {
            cout << authorName << " shared";
        }

        cout << " \"" << p->text << "\"";
        cout << " ...(" << p->date.day << "/" << p->date.month << "/" << p->date.year << ")";
        cout << endl;

        if (p->commentCount > 0) {
            for (int i = 0; i < p->commentCount; i++) {
                Comment* c = p->comments[i];
                cout << "    " << getAuthorName(c->authorId) << " wrote: \"" << c->text << "\"" << endl;
            }
        }
    }

    // ── File Parsing ─────────────────────────

    void loadUsers(const char* filename) {
        ifstream file(filename);
        if (!file.is_open()) return;

        int count;
        file >> count;

        for (int i = 0; i < count; i++) {
            User* usr = new User();
            char uid[20]; file >> uid;
            Helper::safeStrCopy(usr->id, uid, 20);

            char fname[STR_LEN], lname[STR_LEN];
            file >> fname >> lname;
            char fullname[STR_LEN];
            snprintf(fullname, STR_LEN, "%s %s", fname, lname);
            Helper::safeStrCopy(usr->name, fullname);

            char tok[20];
            while (file >> tok && !Helper::strEq(tok, "-1")) {
                usr->addFriend(tok);
            }
            while (file >> tok && !Helper::strEq(tok, "-1")) {
                usr->addLikedPage(tok);
            }
            users[userCount++] = usr;
        }
        file.close();
    }

    void loadPages(const char* filename) {
        ifstream file(filename);
        if (!file.is_open()) return;

        int count;
        file >> count;

        for (int i = 0; i < count; i++) {
            Page* pg = new Page();
            char pid[20]; file >> pid;
            Helper::safeStrCopy(pg->id, pid, 20);

            file >> std::ws;
            file.getline(pg->title, STR_LEN);
            Helper::trim(pg->title);

            pages[pageCount++] = pg;
        }
        file.close();
    }

    void loadPosts(const char* filename) {
        ifstream file(filename);
        if (!file.is_open()) return;

        int count;
        file >> count;

        for (int p = 0; p < count; p++) {
            Post* post = new Post();
            int type; char postId[20];
            file >> type >> postId;
            post->type = type;
            Helper::safeStrCopy(post->id, postId, 20);

            int d, m, y;
            file >> d >> m >> y;
            post->date = Date(d, m, y);

            file >> std::ws;
            file.getline(post->text, STR_LEN);
            Helper::trim(post->text);

            if (type == 2) {
                int actType;
                file >> actType;
                char actVal[STR_LEN];
                file >> std::ws;
                file.getline(actVal, STR_LEN);
                Helper::trim(actVal);
                post->activity.set(actType, actVal);
            }

            char authorId[20];
            file >> authorId;
            Helper::safeStrCopy(post->authorId, authorId, 20);

            char liker[20];
            while (file >> liker && !Helper::strEq(liker, "-1")) {
                post->addLike(liker);
            }

            posts[postCount++] = post;

            if (post->authorId[0] == 'u') {
                User* u = findUser(post->authorId);
                if (u) u->addPost(post);
            }
            else if (post->authorId[0] == 'p') {
                Page* pg = findPage(post->authorId);
                if (pg) pg->addPost(post);
            }
        }
        file.close();
    }

    void loadComments(const char* filename) {
        ifstream file(filename);
        if (!file.is_open()) return;

        int count;
        file >> count;

        for (int i = 0; i < count; i++) {
            char cid[20], pid[20], aid[20];
            file >> cid >> pid >> aid;

            char txt[STR_LEN];
            file >> std::ws;
            file.getline(txt, STR_LEN);
            Helper::trim(txt);

            Comment* cm = new Comment();
            cm->set(cid, pid, aid, txt);
            comments[commentCount++] = cm;

            Post* post = findPost(pid);
            if (post) post->addComment(cm);
        }
        file.close();
    }

    // ── Commands ─────────────────────────────

    void cmdSetDate(int d, int m, int y) {
        systemDate = Date(d, m, y);
        cout << "System Date: " << d << "/" << m << "/" << y << endl;
    }

    void cmdSetUser(const char* uid) {
        User* u = findUser(uid);
        if (!u) { cout << "User not found." << endl; return; }
        currentUser = u;
        cout << u->name << " successfully set as Current User" << endl;
    }

    void cmdViewFriendList() {
        if (!currentUser) { cout << "No current user set." << endl; return; }
        cout << "-----------------------------------------------------------------------------------" << endl;
        cout << currentUser->name << " - Friend List" << endl;
        for (int i = 0; i < currentUser->friendCount; i++) {
            User* f = findUser(currentUser->friendIds[i]);
            cout << currentUser->friendIds[i] << " - " << (f ? f->name : "Unknown") << endl;
        }
        cout << "-----------------------------------------------------------------------------------" << endl;
    }

    void cmdViewLikedPages() {
        if (!currentUser) { cout << "No current user set." << endl; return; }
        cout << "-----------------------------------------------------------------------------------" << endl;
        cout << currentUser->name << " - Liked Pages" << endl;
        for (int i = 0; i < currentUser->likedPageCount; i++) {
            Page* pg = findPage(currentUser->likedPageIds[i]);
            cout << currentUser->likedPageIds[i] << " - " << (pg ? pg->title : "Unknown") << endl;
        }
        cout << "-----------------------------------------------------------------------------------" << endl;
    }

    void cmdViewHome() {
        if (!currentUser) { cout << "No current user set." << endl; return; }
        cout << "-----------------------------------------------------------------------------------" << endl;
        cout << currentUser->name << " - Home Page" << endl;
        bool anyPost = false;

        for (int i = 0; i < postCount; i++) {
            Post* p = posts[i];
            if (p->isMemory) continue;

            bool isFriend = false;
            for (int f = 0; f < currentUser->friendCount; f++) {
                if (Helper::strEq(p->authorId, currentUser->friendIds[f])) { isFriend = true; break; }
            }

            bool isLikedPage = false;
            for (int f = 0; f < currentUser->likedPageCount; f++) {
                if (Helper::strEq(p->authorId, currentUser->likedPageIds[f])) { isLikedPage = true; break; }
            }

            if ((isFriend || isLikedPage) && p->date.isWithin24Hours(systemDate)) {
                printPost(p);
                anyPost = true;
            }
        }
        if (!anyPost) cout << "(No posts in the last 24 hours)" << endl;
        cout << "-----------------------------------------------------------------------------------" << endl;
    }

    void cmdViewProfile() {
        if (!currentUser) { cout << "No current user set." << endl; return; }
        cout << "-----------------------------------------------------------------------------------" << endl;
        cout << currentUser->name << " - Time Line" << endl;
        for (int i = 0; i < postCount; i++) {
            Post* p = posts[i];
            if (Helper::strEq(p->authorId, currentUser->id)) {
                printPost(p);
            }
        }
        cout << "-----------------------------------------------------------------------------------" << endl;
    }

    void cmdViewLikedList(const char* postId) {
        Post* p = findPost(postId);
        if (!p) { cout << "Post not found." << endl; return; }
        cout << "-----------------------------------------------------------------------------------" << endl;
        cout << "Post Liked By:" << endl;
        for (int i = 0; i < p->likeCount; i++) {
            const char* lid = p->likedBy[i];
            cout << lid << " - " << getAuthorName(lid) << endl;
        }
        cout << "-----------------------------------------------------------------------------------" << endl;
    }

    void cmdLikePost(const char* postId) {
        if (!currentUser) { cout << "No current user set." << endl; return; }
        Post* p = findPost(postId);
        if (!p) { cout << "Post not found." << endl; return; }
        if (p->addLike(currentUser->id)) {
            cout << "Post liked successfully." << endl;
        }
        else {
            cout << "Cannot like post." << endl;
        }
    }

    void cmdPostComment(const char* postId, const char* text) {
        if (!currentUser) { cout << "No current user set." << endl; return; }
        Post* p = findPost(postId);
        if (!p) { cout << "Post not found." << endl; return; }

        Comment* c = new Comment();
        char cid[20];
        snprintf(cid, 20, "c%d", commentCount + 1);
        c->set(cid, postId, currentUser->id, text);
        comments[commentCount++] = c;

        if (!p->addComment(c)) {
            cout << "Max comments reached." << endl;
        }
        else {
            cout << "Comment posted." << endl;
        }
    }

    void cmdViewPost(const char* postId) {
        Post* p = findPost(postId);
        if (!p) { cout << "Post not found." << endl; return; }
        printPost(p);
    }

    void cmdSeeYourMemories() {
        if (!currentUser) { cout << "No current user set." << endl; return; }
        cout << "-----------------------------------------------------------------------------------" << endl;
        cout << "We hope you enjoy looking back and sharing your memories on Facebook, from the most recent to those long ago." << endl;
        bool any = false;
        for (int i = 0; i < postCount; i++) {
            Post* p = posts[i];
            if (!Helper::strEq(p->authorId, currentUser->id)) continue;
            if (p->isMemory) continue;
            if (p->date.day == systemDate.day && p->date.month == systemDate.month && p->date.year != systemDate.year) {
                int yearsAgo = systemDate.year - p->date.year;
                cout << "On this Day" << endl;
                cout << yearsAgo << " Year" << (yearsAgo > 1 ? "s" : "") << " Ago" << endl;
                printPost(p);
                any = true;
            }
        }
        if (!any) cout << "(No memories for today)" << endl;
        cout << "-----------------------------------------------------------------------------------" << endl;
    }

    void cmdShareMemory(const char* postId, const char* text) {
        if (!currentUser) { cout << "No current user set." << endl; return; }
        Post* orig = findPost(postId);
        if (!orig) { cout << "Post not found." << endl; return; }

        Post* memory = new Post();
        char mid[20];
        snprintf(mid, 20, "post%d", postCount + 1);
        Helper::safeStrCopy(memory->id, mid, 20);
        memory->date = systemDate;
        memory->isMemory = true;
        Helper::safeStrCopy(memory->originalPostId, postId, 20);
        Helper::safeStrCopy(memory->memoryText, text);
        Helper::safeStrCopy(memory->authorId, currentUser->id, 20);
        memory->type = orig->type;
        memory->activity = orig->activity;
        Helper::safeStrCopy(memory->text, orig->text);

        posts[postCount++] = memory;
        currentUser->addPost(memory);
        cout << "Memory shared successfully." << endl;
    }

    void cmdViewPage(const char* pageId) {
        Page* pg = findPage(pageId);
        if (!pg) { cout << "Page not found." << endl; return; }
        cout << "-----------------------------------------------------------------------------------" << endl;
        cout << pg->title << endl;
        for (int i = 0; i < pg->postCount; i++) {
            printPost(pg->posts[i]);
        }
        cout << "-----------------------------------------------------------------------------------" << endl;
    }

    void showMenu() {
        cout << "\n1. Set System Date | 2. Set Current User | 3. View Home | 4. View Profile | 5. View Friend List" << endl;
        cout << "6. View Liked Pages | 7. View Liked List | 8. Like Post | 9. Post Comment | 10. View Post" << endl;
        cout << "11. See Memories | 12. Share Memory | 13. View Page | 0. Exit" << endl;
        cout << "Command: ";
    }

public:
    SocialNetworkApp() : userCount(0), pageCount(0), postCount(0), commentCount(0), currentUser(nullptr) {
        users = new User * [MAX_USERS];
        pages = new Page * [MAX_PAGES];
        posts = new Post * [MAX_POSTS];
        comments = new Comment * [MAX_COMMENTS];
        for (int i = 0; i < MAX_USERS; i++) users[i] = nullptr;
        for (int i = 0; i < MAX_PAGES; i++) pages[i] = nullptr;
        for (int i = 0; i < MAX_POSTS; i++) posts[i] = nullptr;
        for (int i = 0; i < MAX_COMMENTS; i++) comments[i] = nullptr;
        systemDate = Date(15, 11, 2017);
    }

    ~SocialNetworkApp() {
        for (int i = 0; i < userCount; i++) { delete users[i];    users[i] = nullptr; }
        for (int i = 0; i < pageCount; i++) { delete pages[i];    pages[i] = nullptr; }
        for (int i = 0; i < postCount; i++) { delete posts[i];    posts[i] = nullptr; }
        for (int i = 0; i < commentCount; i++) { delete comments[i]; comments[i] = nullptr; }
        delete[] users;
        delete[] pages;
        delete[] posts;
        delete[] comments;
    }

    void LoadData() {
        loadPages("pages.txt");
        loadUsers("users.txt");
        loadPosts("posts.txt");
        loadComments("comments.txt");
    }

    void Run() {
        LoadData();
        int choice = -1;
        while (choice != 0) {
            showMenu();
            cin >> choice;
            cin.ignore();
            char buf[STR_LEN];

            switch (choice) {
            case 1: {
                int d, m, y;
                cout << "Enter date (dd mm yyyy): ";
                cin >> d >> m >> y; cin.ignore();
                cmdSetDate(d, m, y);
                break;
            }
            case 2:
                cout << "Enter user ID (e.g. u7): ";
                cin.getline(buf, STR_LEN);
                Helper::trim(buf);
                cmdSetUser(buf);
                break;
            case 3: cmdViewHome(); break;
            case 4: cmdViewProfile(); break;
            case 5: cmdViewFriendList(); break;
            case 6: cmdViewLikedPages(); break;
            case 7:
                cout << "Enter post ID (e.g. post5): ";
                cin.getline(buf, STR_LEN);
                Helper::trim(buf);
                cmdViewLikedList(buf);
                break;
            case 8:
                cout << "Enter post ID to like: ";
                cin.getline(buf, STR_LEN);
                Helper::trim(buf);
                cmdLikePost(buf);
                break;
            case 9: {
                char pid[20], text[STR_LEN];
                cout << "Enter post ID: ";
                cin.getline(pid, 20);
                Helper::trim(pid);
                cout << "Enter comment: ";
                cin.getline(text, STR_LEN);
                cmdPostComment(pid, text);
                break;
            }
            case 10:
                cout << "Enter post ID: ";
                cin.getline(buf, STR_LEN);
                Helper::trim(buf);
                cmdViewPost(buf);
                break;
            case 11: cmdSeeYourMemories(); break;
            case 12: {
                char pid[20], text[STR_LEN];
                cout << "Enter post ID to share as memory: ";
                cin.getline(pid, 20);
                Helper::trim(pid);
                cout << "Enter your text: ";
                cin.getline(text, STR_LEN);
                cmdShareMemory(pid, text);
                break;
            }
            case 13:
                cout << "Enter page ID (e.g. p1): ";
                cin.getline(buf, STR_LEN);
                Helper::trim(buf);
                cmdViewPage(buf);
                break;
            case 0: break;
            default: cout << "Invalid choice." << endl;
            }
        }
    }
};

// ─────────────────────────────────────────────
// Main Execution
// ─────────────────────────────────────────────
int main() {
    SocialNetworkApp app;
    app.Run();
    return 0;
}