#include <iostream>           // Standart giriş/çıkış işlemleri için gerekli kütüphane
#include <fstream>           // Dosya okuma işlemleri için gerekli kütüphane
#include <string>           // Metin işlemleri için gerekli kütüphane
#include <vector>          // Dinamik dizi (vektör) işlemleri için gerekli kütüphane
#include <algorithm>      // Algoritma işlemleri için gerekli kütüphane
#include <unordered_map> // Karma tablo (unordered_map) kullanımı için gerekli kütüphane
#include <unordered_set>// Karma kümelerin (unordered_set) kullanımı için gerekli kütüphane
#include <queue>       // Kuyruk (queue) kullanımı için gerekli kütüphane
#include <sqlite3.h>  // SQLite veritabanı işlemleri için gerekli kütüphane
#include <bitset>    //bit kaydırma-sıkıştırma işlemleri için

//g++ Huffman.cpp -o program -lsqlite3(sql'i programa dahil etmek için '.cpp dosyası'+'oluşturulacak exe adı')
using namespace std;//standart std kullanımı

struct HuffmanNode {//Huffman ağacı için düğüm yapımız
    int frequency;
    char character;
    string code;
    HuffmanNode* left;
    HuffmanNode* right;

    bool operator<(const HuffmanNode& other) const {
        return frequency > other.frequency;//küçüktür operatörünü iki düğümün frekans boyutunu kıyasllayacak biçimde çalışması için aşırı yüklüyoruz
    }
};

void deleteHuffmanTree(HuffmanNode* node) {//new ile tahsis edilmiş düğüm belleğini rekörsif olarak siliyoruz(free ediyoruz)
    if (node == nullptr) {
        return;
    }
    deleteHuffmanTree(node->left);
    deleteHuffmanTree(node->right);
    delete node;
}

HuffmanNode* buildHuffmanTree(const unordered_map<char, int>& frequencyMap) {//öncelik kuyruğuna Huffman düğümlerimizi ekliyoruz(priority queue tipi diziye)
    priority_queue<HuffmanNode*> pq;
    for (auto it = frequencyMap.begin(); it != frequencyMap.end(); ++it) {
        HuffmanNode* node = new HuffmanNode();
        node->frequency = it->second;
        node->character = it->first;
        node->code = "";
        node->left = nullptr;
        node->right = nullptr;
        pq.push(node);// unordered map yani karmaşık arrayin char kısmı ilk(first) int kısmı ikinci(second) pointleme şeklinde tanımlanıyor 
    }

    while (pq.size() > 1) {
        HuffmanNode* left = pq.top();//öncelik kuyruğundan ikili ikili düğüm atımı ve ebeveyn düğümü eklenme işini görüyoruz
        pq.pop();

        HuffmanNode* right = pq.top();
        pq.pop();

        HuffmanNode* parent = new HuffmanNode();
        parent->frequency = left->frequency + right->frequency;
        parent->character = '\0';
        parent->code = "";
        parent->left = left;
        parent->right = right;

        pq.push(parent);
    }

    return pq.top();
}

void generateHuffmanCodes(HuffmanNode* root, unordered_map<char, string>& huffmanCodes) {//buildHuffmanTree fonksiyonu ile oluşturduğumuz ağaçtan yaprak düğümlere inerek;
    if (root == nullptr)
        return;

    if (root->left == nullptr && root->right == nullptr) {//her sol için  bir 0 her sağ için 1 ekleyerek huffman kodunu oluşturuyoruz
        huffmanCodes[root->character] = root->code;
        return;
    }

    root->left->code = root->code + "0";
    root->right->code = root->code + "1";

    generateHuffmanCodes(root->left, huffmanCodes);
    generateHuffmanCodes(root->right, huffmanCodes);
}

string encodeText(const string& text, const unordered_map<char, string>& huffmanCodes) {//harf başına ürettiğimiz bit kodları ile tam huffman metnini üretiyoruz
    string encodedText;
    for (char ch : text) {
        encodedText += huffmanCodes.at(ch);
    }
    return encodedText;
}

string compressText(const std::string& encodedText) {//huffman metnindeki her 8 bit için bir ascii karakter üretiyor,(8'in katı değilse)
    std::string compressedMsg;                      //8'e tamamlayacak kadar 0 ekliyor ancak.sorun-->@decompressText
    std::string paddedEncodedText = encodedText;

    if (paddedEncodedText.length() % 8 != 0) {
        int tni = 8 - paddedEncodedText.length() % 8;

        for (int i = 0; i < tni; i++) {
            paddedEncodedText += '0';
        }
    }

    for (size_t i = 0; i < paddedEncodedText.length(); i += 8) {
        std::string substring = paddedEncodedText.substr(i, 8);

        char asciiChar = 0;
        for (size_t j = 0; j < substring.length(); j++) {
            asciiChar = (asciiChar << 1) + (substring[j] - '0');
        }

        compressedMsg += asciiChar;
    }

    return compressedMsg;
}

string decompressText(const string& compressedText, HuffmanNode* huffmanTree)//hafızadaki (sql değil program hafızası)bit kodlarına göre 
 {                                                                          //içine girdiğiniz "sıkıştırılmış"metni açıyor ancak bir kusuru var
    string decompressedText;                                               //eğer ilk metninizi geri üretmeye çalışırsanız,sıkıştırmak için eklediği
                                                                          // fazlalık 0'ları silmiyor(silmek adına bir yol bulamadım) yani fazla veri de vermiş
    string binaryCode;                                                   //olsa idealimiz olan kayıpsız veri sıkıştırmada başarısız olmakta bu hatayı bir nebze 
                                                                        //telafi etmesi için sonuç ascii kodu hariç her şey bir veri yapısına kaydedilmektedir
    for (char ch : compressedText) {                                   //açmak adına bir db okur indirmenizi tavsiye ederim
        binaryCode += bitset<8>(ch).to_string();
    }

    size_t codeLength = 0;
    HuffmanNode* currentNode = huffmanTree;

    while (codeLength < binaryCode.length()) {
        if (currentNode->left == nullptr && currentNode->right == nullptr) {
            decompressedText += currentNode->character;
            currentNode = huffmanTree;
        }

        if (binaryCode[codeLength] == '0') {
            currentNode = currentNode->left;
        } else if (binaryCode[codeLength] == '1') {
            currentNode = currentNode->right;
        }

        codeLength++;
    }

    if (currentNode->left == nullptr && currentNode->right == nullptr) {
        decompressedText += currentNode->character;
    }

    return decompressedText;
}

void printHuffmanTable(const unordered_map<char, int>& frequencyMap, const unordered_map<char, string>& huffmanCodes) {
    cout << "Karakter\tFrekans\t\tHuffman Kodu" << endl;
    for (auto it = frequencyMap.begin(); it != frequencyMap.end(); ++it) {
        char ch = it->first;
        int frequency = it->second;
        string code = huffmanCodes.at(ch);
        cout << ch << "\t\t" << frequency << "\t\t" << code << endl;//harf kodunu,harfin frekansını ve harfi bastırır
    }
}

int main() {//main işlev-ilgili girdi kısımları(database ismi txt ismi gibi //ilki .db uzantılı olmayan bir dosya ikincisi .txt uzantılı var olan
    string dbName;//bir dosya olmalı)
    cout << "Veritabani adini girin: ";
    getline(cin, dbName);

    string fileName;
    cout << "Dosya adini girin: ";
    getline(cin, fileName);

    ifstream inputFile(fileName);
    if (!inputFile) {
        cout << "Dosya acilamadi." << endl;
        return 1;
    }

    string content;
    string line;
    while (getline(inputFile, line)) {
        content += line;
    }

    unordered_map<char, int> frequencyMap;
    for (char ch : content) {
        frequencyMap[ch]++;
    }

    vector<HuffmanNode*> nodes;

    for (auto it = frequencyMap.begin(); it != frequencyMap.end(); ++it) {
        HuffmanNode* node = new HuffmanNode();
        node->frequency = it->second;
        node->character = it->first;
        node->code = "";
        node->left = nullptr;
        node->right = nullptr;
        nodes.push_back(node);
    }

    while (nodes.size() > 1) {
        sort(nodes.begin(), nodes.end(), [](const HuffmanNode* a, const HuffmanNode* b) {
            return a->frequency > b->frequency;
        });

        HuffmanNode* left = nodes.back();
        nodes.pop_back();

        HuffmanNode* right = nodes.back();
        nodes.pop_back();

        HuffmanNode* parent = new HuffmanNode();
        parent->frequency = left->frequency + right->frequency;
        parent->character = '\0';
        parent->code = "";
        parent->left = left;
        parent->right = right;

        nodes.push_back(parent);
    }

    HuffmanNode* huffmanTree = nodes[0];
    unordered_map<char, string> huffmanCodes;
    generateHuffmanCodes(huffmanTree, huffmanCodes);
    unordered_set<char> printedChars;

    printHuffmanTable(frequencyMap, huffmanCodes);

    cout << "Girilen Metin: " << content << endl;
    string encodedText = encodeText(content, huffmanCodes);
    cout << "Metnin Huffman Hali: " << encodedText << endl;
    string compressedMsg = compressText(encodedText);
    cout << "Compressed Message: " << compressedMsg << endl;

    ofstream outputFile(fileName);
    if (!outputFile) {
        cout << "Dosya olusturulamadi." << endl;
        return 1;
    }

    outputFile << compressedMsg;
    outputFile.close();

    sqlite3* db;//veri tabanına bağlanma kısmı
    int rc = sqlite3_open(dbName.c_str(), &db);
    if (rc) {
        cout << "Veritabanina baglanti saglanamadi: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    string createTableQuery = "CREATE TABLE IF NOT EXISTS HuffmanTable ("//veri tabanı tablosu oluşturmak adına sorgular
                              "Character CHAR(1) PRIMARY KEY,"
                              "Frequency INT,"
                              "Code TEXT,"
                              "Text TEXT,"
                              "EncodedText TEXT,"
                              "ZeroCount INT);";
    rc = sqlite3_exec(db, createTableQuery.c_str(), 0, 0, 0);
    if (rc) {
        cout << "Tablo olusturma hatasi: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return 1;
    }

    for (auto it = huffmanCodes.begin(); it != huffmanCodes.end(); ++it) {
        char ch = it->first;
        int frequency = frequencyMap[ch];
        string code = it->second;

        int zeroCount = count(code.begin(), code.end(), '0');

        string insertQuery = "INSERT INTO HuffmanTable (Character, Frequency, Code, Text, EncodedText, ZeroCount) "
                             "VALUES ('" + string(1, ch) + "', " + to_string(frequency) + ", '" + code + "', '" + content + "', '" + encodedText + "', " + to_string(zeroCount) + ");";
        rc = sqlite3_exec(db, insertQuery.c_str(), 0, 0, 0);
        if (rc) {
            cout << "Veri ekleme hatasi: " << sqlite3_errmsg(db) << endl;
            sqlite3_close(db);
            return 1;
        }
    }
    
    
    
    string decompressedText = decompressText(compressedMsg, huffmanTree);
    cout<<"Decompressed Message: "<<decompressedText<<endl; //deneme yeri
    
    sqlite3_close(db);
    inputFile.close();

    deleteHuffmanTree(huffmanTree);//hafıza boşalta

    return 0;
}