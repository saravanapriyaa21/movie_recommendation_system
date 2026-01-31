#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <chrono>

using namespace std;

/* =========================
   HELPERS
   ========================= */
bool isNumber(const string& s) {
    return !s.empty() &&
           all_of(s.begin(), s.end(), [](char c) { return isdigit(c); });
}

/* =========================
   DATA STRUCTURES
   ========================= */
struct User {
    int id;
    string password;
};

struct Movie {
    string name;
    string genre;
};

/* =========================
   GLOBAL STATE
   ========================= */
vector<pair<int, string>> user_watch_history; // movie_id, name
unordered_map<int, tuple<string, string, double>> movieGenres;

/* =========================
   CSV READER (ROBUST)
   ========================= */
vector<vector<string>> readCSV(const string& filename) {
    vector<vector<string>> data;
    ifstream file(filename);
    if (!file) return data;

    string line;
    getline(file, line); // skip header

    while (getline(file, line)) {
        vector<string> row;
        string cell;
        bool quotes = false;

        for (char c : line) {
            if (c == '"') quotes = !quotes;
            else if (c == ',' && !quotes) {
                row.push_back(cell);
                cell.clear();
            } else {
                cell += c;
            }
        }
        row.push_back(cell);
        data.push_back(row);
    }
    return data;
}

/* =========================
   LOADERS (SAFE)
   ========================= */
unordered_map<string, User> loadUsers(const string& file) {
    unordered_map<string, User> users;
    auto data = readCSV(file);

    for (auto &r : data) {
        if (r.size() < 3) continue;
        if (!isNumber(r[0])) continue;
        users[r[1]] = {stoi(r[0]), r[2]};
    }
    return users;
}

unordered_map<int, Movie> loadMovies(const string& file) {
    unordered_map<int, Movie> movies;
    auto data = readCSV(file);

    for (auto &r : data) {
        if (r.size() < 3) continue;
        if (!isNumber(r[0])) continue;
        movies[stoi(r[0])] = {r[1], r[2]};
    }
    return movies;
}

unordered_map<int, set<int>> loadWatchHistoryMap(const string& file) {
    unordered_map<int, set<int>> wh;
    auto data = readCSV(file);

    for (auto &r : data) {
        if (r.size() < 2) continue;
        if (!isNumber(r[0]) || !isNumber(r[1])) continue;
        wh[stoi(r[0])].insert(stoi(r[1]));
    }
    return wh;
}

/* =========================
   MAPREDUCE: TOP RATED
   ========================= */
unordered_map<int, vector<double>> mapRatings(const vector<vector<string>>& chunk) {
    unordered_map<int, vector<double>> local;
    for (auto &r : chunk) {
        if (r.size() < 4) continue;
        if (!isNumber(r[0])) continue;
        if (r[3] == "NULL" || r[3].empty()) continue;

        try {
            local[stoi(r[0])].push_back(stod(r[3]));
        } catch (...) {}
    }
    return local;
}

unordered_map<int, vector<double>> shuffleRatings(
    const vector<unordered_map<int, vector<double>>>& partials) {

    unordered_map<int, vector<double>> merged;
    for (auto &p : partials)
        for (auto &e : p)
            merged[e.first].insert(
                merged[e.first].end(),
                e.second.begin(), e.second.end()
            );
    return merged;
}

unordered_map<int, double> reduceAverageRatings(
    const unordered_map<int, vector<double>>& ratings) {

    unordered_map<int, double> avg;
    for (auto &e : ratings) {
        double sum = 0;
        for (double r : e.second) sum += r;
        avg[e.first] = sum / e.second.size();
    }
    return avg;
}

/* =========================
   COLLABORATIVE FILTERING
   ========================= */
double jaccard(const set<int>& a, const set<int>& b) {
    set<int> i, u;
    set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter(i, i.begin()));
    set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(u, u.begin()));
    return u.empty() ? 0.0 : (double)i.size() / u.size();
}

vector<int> recommendCollaborative(
    int uid, const unordered_map<int, set<int>>& wh) {

    vector<pair<int,double>> scores;

    for (auto &u : wh) {
        if (u.first == uid) continue;
        double s = jaccard(wh.at(uid), u.second);
        if (s > 0) scores.push_back({u.first, s});
    }

    sort(scores.begin(), scores.end(),
         [](auto &a, auto &b){ return a.second > b.second; });

    if (scores.size() > 2) scores.resize(2);

    map<int,double> movieScore;
    for (auto &u : scores)
        for (int m : wh.at(u.first))
            if (!wh.at(uid).count(m))
                movieScore[m] += u.second;

    vector<pair<int,double>> sorted(movieScore.begin(), movieScore.end());
    sort(sorted.begin(), sorted.end(),
         [](auto &a, auto &b){ return a.second > b.second; });

    vector<int> recs;
    for (auto &m : sorted) {
        recs.push_back(m.first);
        if (recs.size() == 10) break;
    }
    return recs;
}

/* =========================
   GENRE BASED
   ========================= */
void buildUserHistory(int uid) {
    user_watch_history.clear();
    movieGenres.clear();

    auto movies = readCSV("movie.csv");
    auto watch = readCSV("watch_history.csv");

    for (auto &m : movies) {
        if (m.size() < 4) continue;
        if (!isNumber(m[0])) continue;

        double rating = 0.0;
        try { rating = stod(m[3]); } catch (...) {}

        movieGenres[stoi(m[0])] = {m[2], m[1], rating};
    }

    for (auto &w : watch) {
        if (w.size() < 3) continue;
        if (!isNumber(w[0]) || !isNumber(w[1])) continue;

        if (stoi(w[0]) == uid)
            user_watch_history.push_back({stoi(w[1]), w[2]});
    }
}

string mostWatchedGenre() {
    map<string,int> count;
    for (auto &p : user_watch_history)
        count[get<0>(movieGenres[p.first])]++;

    if (count.empty()) return "";
    return max_element(count.begin(), count.end(),
        [](auto &a, auto &b){ return a.second < b.second; })->first;
}

/* =========================
   SURPRISE
   ========================= */
void surpriseMe(int uid,
    const unordered_map<int,set<int>>& wh,
    const unordered_map<int,Movie>& movies) {

    vector<int> pool;
    for (auto &m : movies)
        if (!wh.at(uid).count(m.first))
            pool.push_back(m.first);

    shuffle(pool.begin(), pool.end(), mt19937(random_device{}()));

    cout << "\nEXPLORE THE UNEXPLORED:\n\n";
    for (int i = 0; i < 10 && i < pool.size(); i++)
        cout << "- " << movies.at(pool[i]).name << "\n";
}

/* =========================
   MAIN
   ========================= */
int main() {

    auto users = loadUsers("users.csv");
    auto movies = loadMovies("movies.csv");
    auto watchMap = loadWatchHistoryMap("watch_history.csv");

    string user, pass;
    cout << "Username: "; cin >> user;
    cout << "Password: "; cin >> pass;

    if (!users.count(user) || users[user].password != pass) {
        cout << "Invalid login\n";
        return 0;
    }

    int uid = users[user].id;
    cout << "\nWelcome " << user << "!\n";

    while (true) {
        cout << "\n1. Top Rated Movies\n"
             << "2. Watch History Recommendations\n"
             << "3. Genre Based Recommendations\n"
             << "4. Surprise Me\n"
             << "5. Exit\n"
             << "Choice: ";

        int ch; cin >> ch;
        auto start = chrono::high_resolution_clock::now();

        if (ch == 1) {
            auto data = readCSV("movie.csv");
            vector<vector<string>> chunks[4];
            for (int i = 0; i < data.size(); i++)
                chunks[i % 4].push_back(data[i]);

            vector<unordered_map<int,vector<double>>> partials;
            for (int i = 0; i < 4; i++)
                partials.push_back(mapRatings(chunks[i]));

            auto ratings = reduceAverageRatings(shuffleRatings(partials));
            vector<pair<int,double>> sorted(ratings.begin(), ratings.end());
            sort(sorted.begin(), sorted.end(),
                 [](auto &a, auto &b){ return a.second > b.second; });

            cout << "\nTOP RATED MOVIES:\n\n";
            for (int i = 0; i < 10 && i < sorted.size(); i++)
                cout << "- " << movies[sorted[i].first].name
                     << " (" << sorted[i].second << ")\n";
        }

        else if (ch == 2) {
            auto recs = recommendCollaborative(uid, watchMap);
            cout << "\nRECOMMENDATIONS:\n\n";
            if (recs.empty())
                cout << "No strong similarity found.\n";
            else
                for (int m : recs)
                    cout << "- " << movies[m].name << "\n";
        }

        else if (ch == 3) {
            buildUserHistory(uid);
            string g = mostWatchedGenre();

            if (g.empty()) {
                cout << "No watch history available.\n";
            } else {
                cout << "\nFAVOURITE GENRE: " << g << "\n\n";
                vector<pair<double,string>> genreMovies;

                for (auto &m : movieGenres)
                    if (get<0>(m.second) == g)
                        genreMovies.push_back({get<2>(m.second), get<1>(m.second)});

                sort(genreMovies.rbegin(), genreMovies.rend());
                for (int i = 0; i < 10 && i < genreMovies.size(); i++)
                    cout << "- " << genreMovies[i].second << "\n";
            }
        }

        else if (ch == 4) {
            surpriseMe(uid, watchMap, movies);
        }

        else if (ch == 5) break;

        auto end = chrono::high_resolution_clock::now();
        cout << "\nCompleted in "
             << chrono::duration_cast<chrono::milliseconds>(end-start).count()
             << " ms\n";
    }

    return 0;
}
