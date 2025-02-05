#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
using namespace std;

class TextBox;

class TextTable
{
public:
    TextTable(const string& csvPath);
    ~TextTable();

    void LoadCSV(const string& filePath);

    void SetLanguage(const string& language);

    string GetString(const string& id) const;

    void SetStringProperty(const string& key, const string& value);

    string Substitute(const string& input) const;

    void RegisterOnChange(function<void()> observer) {
        m_observers.push_back(observer);
    }

private:
    void Notify() {
        for(auto& obs : m_observers) {
            obs();
        }
    }

private:
    unordered_map<string, unordered_map<string, string>> m_strings;
    string m_currentLanguage;

    unordered_map<string, string> m_substitutions;

    vector<function<void()>> m_observers;
};
