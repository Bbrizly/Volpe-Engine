#include "TextTable.h"

TextTable::TextTable(const string& path)
{
    LoadCSV(path);
    // default or empty language
}

TextTable::~TextTable()
{
}

void TextTable::LoadCSV(const string& filePath)
{
    ifstream file(filePath);
    if(!file.is_open()) {
        throw runtime_error("Failed to open CSV file: " + filePath);
    }

    // read header line
    string headerLine;
    if(!getline(file, headerLine)) {
        throw runtime_error("CSV file empty or invalid: " + filePath);
    }

    // split header
    vector<string> headers;
    {
        stringstream ss(headerLine);
        string col;
        while(getline(ss, col, ',')) {
            headers.push_back(col);
        }
    }
    if(headers.size() < 2) {
        throw runtime_error("CSV must have at least 2 columns (ID + at least 1 language).");
    }

    // read rows
    string row;
    while(getline(file, row)) {
        if(row.empty()) continue;

        vector<string> cols;
        {
            stringstream ss(row);
            string c;
            while(getline(ss, c, ',')) {
                cols.push_back(c);
            }
        }
        if(cols.size() < headers.size()) {
            cerr << "Skipping CSV row with fewer columns: " << row << endl;
            continue;
        }

        string id = cols[0];
        // fill language => text
        for(size_t i=1; i<headers.size(); i++) {
            string lang = headers[i];
            string val  = cols[i];
            m_strings[lang][id] = val;
        }
    }

    file.close();
}

void TextTable::SetLanguage(const string& language)
{
    if(m_strings.find(language)==m_strings.end()) {
        throw runtime_error("Language not found in table: " + language);
    }
    m_currentLanguage = language;
    Notify(); // notify watchers
}

string TextTable::GetString(const string& id) const
{
    if(m_currentLanguage.empty()) {
        throw runtime_error("No current language set in TextTable");
    }
    auto langIt = m_strings.find(m_currentLanguage);
    if(langIt == m_strings.end()) {
        throw runtime_error("TextTable: language not loaded: " + m_currentLanguage);
    }
    auto strIt = langIt->second.find(id);
    if(strIt == langIt->second.end()) {
        throw runtime_error("TextTable: ID not found: " + id + " in lang: " + m_currentLanguage);
    }
    // do placeholders
    return Substitute(strIt->second);
}

void TextTable::SetStringProperty(const string& key, const string& value)
{
    m_substitutions[key] = value;
    Notify(); // something changed
}

string TextTable::Substitute(const string& input) const
{
    string out = input;
    for(const auto& kv : m_substitutions) {
        string placeholder = "{" + kv.first + "}";
        size_t pos=0;
        while((pos = out.find(placeholder,pos)) != string::npos) {
            out.replace(pos, placeholder.size(), kv.second);
            pos += kv.second.size();
        }
    }
    return out;
}
