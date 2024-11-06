#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <queue>
#include <filesystem>  

using namespace std;
namespace fs = std::filesystem;

// Common words to ignore
set<string> stopWords = {"A", "AND", "AN", "OF", "IN", "THE"};

// Function to normalize text (uppercase and remove non-alphanumeric characters)
string normalizeWord(const string &word) {
    string normalized;
    for (char ch : word) {
        if (isalnum(ch)) {
            normalized += toupper(ch);
        }
    }
    return normalized;
}

// Function to calculate word frequencies from book content
unordered_map<string, double> calculateTop100Words(const string &content) {
    unordered_map<string, int> wordCount;
    string word;
    int totalWords = 0;
    istringstream stream(content);

    while (stream >> word) {
        word = normalizeWord(word);
        if (stopWords.find(word) == stopWords.end()) {
            wordCount[word]++;
            totalWords++;
        }
    }

    // Convert frequencies to normalized values
    vector<pair<int, string>> freqWords;
    for (const auto &entry : wordCount) {
        freqWords.push_back({entry.second, entry.first});
    }

    // Sort by frequency and select top 100
    sort(freqWords.rbegin(), freqWords.rend());
    if (freqWords.size() > 100) {
        freqWords.resize(100);
    }

    unordered_map<string, double> normalizedFrequency;
    for (const auto &entry : freqWords) {
        normalizedFrequency[entry.second] = entry.first / (double)totalWords;
    }

    return normalizedFrequency;
}

// Function to calculate similarity between two files
double calculateSimilarity(const unordered_map<string, double> &freq1, const unordered_map<string, double> &freq2) {
    double similarity = 0.0;
    for (const auto &entry : freq1) {
        if (freq2.find(entry.first) != freq2.end()) {
            similarity += entry.second + freq2.at(entry.first);
        }
    }
    return similarity;
}

int main() {
    vector<string> bookContents;
    vector<string> fileNames;

    // Folder path
    string folderPath = "/Users/rajatkumar/Desktop/EDUCATION/masai school coding/similiarity project/Book-Txt";

    // Read all .txt files in the folder
    for (const auto &entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".txt") {
            fileNames.push_back(entry.path().string());

            ifstream file(entry.path());
            if (file.fail()) {
                cout << "Error opening " << entry.path() << endl;
                continue;
            }
            
            // Read the whole file into a string
            string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            bookContents.push_back(content);
            file.close();
        }
    }

    int numFiles = bookContents.size();
    vector<unordered_map<string, double>> frequencies(numFiles);
    vector<vector<double>> similarityMatrix(numFiles, vector<double>(numFiles, 0));

    // Process each book content to calculate word frequencies
    for (int i = 0; i < numFiles; i++) {
        frequencies[i] = calculateTop100Words(bookContents[i]);
    }

    // Calculate similarity matrix
    for (int i = 0; i < numFiles; i++) {
        for (int j = i + 1; j < numFiles; j++) {
            similarityMatrix[i][j] = calculateSimilarity(frequencies[i], frequencies[j]);
            similarityMatrix[j][i] = similarityMatrix[i][j]; // Symmetric matrix
        }
    }

    // Priority queue to store top 10 similar pairs
    priority_queue<pair<double, pair<int, int>>> topPairs;

    for (int i = 0; i < numFiles; i++) {
        for (int j = i + 1; j < numFiles; j++) {
            if (topPairs.size() < 10) {
                topPairs.push({similarityMatrix[i][j], {i, j}});
            } else if (similarityMatrix[i][j] > topPairs.top().first) {
                topPairs.pop();
                topPairs.push({similarityMatrix[i][j], {i, j}});
            }
        }
    }

    // Output the top 10 similar pairs
    vector<pair<double, pair<int, int>>> result;
    while (!topPairs.empty()) {
        result.push_back(topPairs.top());
        topPairs.pop();
    }
    reverse(result.begin(), result.end()); // Display in descending order

    cout << "Top 10 similar pairs:" << endl;
    for (const auto &pair : result) {
        cout << "File " << fileNames[pair.second.first] << " and File " 
             << fileNames[pair.second.second] << " with similarity " 
             << pair.first << endl;
    }

    return 0;
}
