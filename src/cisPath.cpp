#define GCC_VERSION (__GNUC__*10000+__GNUC_MINOR__*100+__GNUC_PATCHLEVEL__)
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <list>

#if GCC_VERSION > 40600
#include <tr1/unordered_map>
#else
#include <ext/hash_map>
#endif

#ifdef INDEP_PROGRAM
#include <unistd.h>
#define PRINTFUNCTION printf
#else
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <R_ext/Visibility.h>
#define PRINTFUNCTION Rprintf
#endif

#define INF 1<<20
using namespace std;

#if GCC_VERSION > 40600
#define HASHMAP unordered_map
using namespace std::tr1;
#else
#define HASHMAP hash_map
using namespace __gnu_cxx;
#endif
//////////////////////
struct str_hash1
{
    size_t operator()(const string &str) const
    {
        unsigned long __h = 0;
        for (size_t i = 0 ; i < str.size() ; i ++)
        {
            __h = 107 * __h + str[i];
        }
        return size_t(__h);
    }
};
class Node
{
public:
    string name;
    vector <string> prev;
    int dist;
    bool operator<(const Node &t2) const
    {
        if(dist == t2.dist)
        {
            return name > t2.name;
        }
        else
        {
            return dist > t2.dist;
        }
    }
};
//////////////////////
bool cispath(const char *input, const char *protein, const char *output);
bool viewGraph(const char *input, const char *proteinFile, const char *output,
               int addChilds, const char *childCol);
bool processInput(const char *input);
vector<string> string_tokenize(const string &str,
                               const string &delimiters = " \t\n\r",
                               bool skip_empty = true);
string get_package_bin_path(string path);
bool cp_html(string path, string outputDir);
string trim(string &s);
/*
map<string, string> name2prot;
map<string, string> prot2real;
map<string, map<string,int> > edge;
map<string, string> edgeInfo;
vector<string> nodes;
map<string, int> nodesExist;

map<string, int> dist;
map<string, string> prev;
vector<string> hasPath;*/

HASHMAP<string, string, str_hash1> name2prot;
HASHMAP<string, string, str_hash1> prot2real;
HASHMAP<string, map<string, int>, str_hash1> edge;
HASHMAP<string, string, str_hash1> edgeInfo;
HASHMAP<string, string, str_hash1> edgeInfo_string;
vector<string> nodes;
HASHMAP<string, int, str_hash1> nodesExist;
HASHMAP<string, int, str_hash1> dist;
HASHMAP<string, string, str_hash1> prev;
vector<string> hasPath;

bool withPubmed = false;
bool makedir(const char *dir, int mask = 0744);
string root;
string outputDir;
fstream OUTJSALL;
fstream OUTJS1;
fstream OUTJS2;
string n2pFile = "";
string targetFile = "";
HASHMAP<string, int, str_hash1> targets;
int maxNum = 10000;
int outputPath = 1;

///////////////////////////////////
vector<string> OutPutID2names;
HASHMAP<string, int, str_hash1> name2OutPutID;
int outputID = 0;
////////////////////g++ cisPath.cpp -o cisPath -DINDEP_PROGRAM
#ifdef INDEP_PROGRAM
void print_usage()
{
    cerr << "cispath <PPIFile> <protein> <outputDir>";
    cerr << " [-n name2prot.txt] [-t targets.txt]" << endl;

    cerr <<"----------------------------------------------------------"<<endl;
    cerr <<"  PPIFile: file that contains PPI data for proteins" << endl;
    cerr << "  protein: protein name or Swiss-Prot accession number";
    cerr << " for source protein" << endl;
    cerr << "outputDir: output directory" << endl;
    cerr <<"----------------------------------------------------------"<<endl;
    cerr << " optional:" << endl;
    cerr << "   -n name2prot.txt:";
    cerr << " additional ID mapping information file" << endl;
    cerr << "   -t targets.txt: output paths only for proteins";
    cerr << " listed in this file (default: all)\n";
    cerr << " (gcc version: " << GCC_VERSION << ")" << endl;
    cerr<< "----------------------------------------------------------"<<endl;
    exit(1);
}

string infoFile;
string proteinName;
string outputDirName;
int main(int argc, char *argv[])
{
    if(argc < 4)
    {
        print_usage();
    }
    infoFile = argv[1];
    proteinName = argv[2];
    outputDirName = argv[3];
    n2pFile = "";
    targetFile = "";
    opterr = 0;
    int result;
    while((result = getopt(argc, argv, "n:t:")) != -1 )
    {
        switch(result)
        {
        case 'n':
            printf("additional ID mapping information file: %s\n", optarg);
            n2pFile = optarg;
            break;
        case 't':
            printf("target protein name list file: %s\n", optarg);
            targetFile = optarg;
            break;
        case '?':
            printf("invalid option: -%c\n", optopt);
            print_usage();
            break;
        default:
            printf("invalid option!\n");
            print_usage();
            break;
        }
    }
    makedir(outputDirName.c_str());
    makedir((outputDirName + "/js").c_str());
    makedir((outputDirName + "/D3").c_str());
    string package_path = get_package_bin_path(argv[0]);
    cp_html(package_path, outputDirName);
    cispath(infoFile.c_str(), proteinName.c_str(), outputDirName.c_str());
}
#else
#endif
bool detectPath(string root);
bool showPath(string root);
bool getTargets(string outputDir);
bool addInfoFirst();
bool cispath(const char *input, const char *protein, const char *output)
{
    //PRINTFUNCTION("gcc version: %d\n", GCC_VERSION);
    addInfoFirst();
    PRINTFUNCTION("Processing input file\n");
    PRINTFUNCTION("input file: %s\n", input);
    PRINTFUNCTION("source protein: %s\n", protein);
    PRINTFUNCTION("output directory: %s\n", output);
    outputDir = output;
#ifdef INDEP_PROGRAM
#else
    R_FlushConsole();
#endif
    //return true;
    /*OUT.open(output, ios::out);
    if(!OUT){
       PRINTFUNCTION("Can not open file:%s\n", output);
       return false;
    }*/
    if(!processInput(input))
    {
        return false;
    }
    bool vTarget = getTargets(output);
    if(!vTarget)
    {
#ifdef INDEP_PROGRAM
        cerr << "These are no valid target protein names!" << endl;
#else
        PRINTFUNCTION("These are no valid target protein names!\n");
        R_FlushConsole();
#endif
        return false;
    }
    string root_prot = protein;
    if(name2prot.find(root_prot) != name2prot.end())
    {
        root_prot = name2prot[root_prot];
#ifdef INDEP_PROGRAM
        cerr << protein << ": valid protein name" << endl;
        cerr << "Swiss-Prot number: " << root_prot << endl;
#else
        PRINTFUNCTION("%s: valid protein name\n", protein);
        PRINTFUNCTION("Swiss-Prot number: %s\n", root_prot.c_str());
        R_FlushConsole();
#endif
    }
    else
    {
#ifdef INDEP_PROGRAM
        cerr << protein << ": can not be found in the name list " << endl;
        cerr << protein << ": will be treated as a Swiss-Prot number" << endl;
#else
        PRINTFUNCTION("%s: can not be found in the name list\n", protein);
        PRINTFUNCTION("%s: will be treated as a Swiss-Prot number\n", protein);
        R_FlushConsole();
#endif
    }
    root = root_prot;
    detectPath(root_prot);
    showPath(root_prot);
    return true;
}
string path;
string nodeStr;
string linkStr;
map<string, int> nodeName2id;
map<string, int> link2id;
int id = 0;
vector<string> childs;
vector<string> mainNodes;
int count2 = 0;

bool addLink(string source, string target, int group, int bigNode);
bool addLink2(string name);
bool addChildLink(string source, string target);
bool addNode(string Name, int group);
bool addChildLinks(vector<string> &allChilds, vector<string> &mainNodes);
bool addRootLinks(vector<string> &allChilds, vector<string> &mainNodes);

vector<string> onePath;
int pathNum = 0;
bool printPath()
{
    if(onePath.size() == 0)
    {
        return true;
    }
    nodeName2id.clear();
    link2id.clear();
    id = 0;
    nodeStr = "";
    linkStr = "";
    childs.clear();
    mainNodes.clear();
    string current = onePath[0];
    int index = 0;
    string realName = current;
    if(prot2real.count(current) != 0)
    {
        realName = prot2real[current];
    }
    realName = realName + ":" + current;
    path = realName;
    while(current != root)
    {
        addLink(current, onePath[index + 1], -1, 1);
        if(nodeName2id.count(current) == 0)
        {
            PRINTFUNCTION("Thers is something wrong! Position6\n");
            return false;
        }
        addLink2(current);
        if(current == onePath[index + 1])
        {
            PRINTFUNCTION("Thers is something wrong! Position7\n");
            return false;
        }
        current = onePath[index + 1];
        index++;
        string realName = current;
        if(prot2real.count(realName) > 0)
        {
            realName = prot2real[realName];
        }
        realName = realName + ":" + current;
        path = realName + " -> " + path;
    }
    addNode(root, -1);
    addLink2(root);
    addChildLinks(childs, mainNodes);
    addRootLinks(childs, mainNodes); //only for path from root to root
    if((onePath.size() >= 4) && (pathNum > maxNum))
    {
        outputPath = 0;
    }
    if((pathNum >= 0) && (outputPath != 0))
    {
        OUTJS1 << "allPaths[" << pathNum << "]=";
        OUTJS1 << "\"" << path << "\"" << ";\n";
        OUTJS2 << "allGraphs[" << pathNum << "]=";
        OUTJS2 << "\"" + nodeStr + "#" + linkStr + "\";\n";
    }
    pathNum++;
    return true;
}

void getPath(string target)
{
    onePath.push_back(target);
    if(target == root)
    {
        printPath();
        onePath.pop_back();
        return;
    }
    if(outputPath == 0)
    {
        onePath.pop_back();
        return;
    }
    string info = prev[target];
    vector<string> tokens = string_tokenize(info, "#", false);
    for(int i = 0; i < (int)tokens.size(); i++)
    {
        getPath(tokens[i]);
    }
    onePath.pop_back();
}

bool showPath(string root)
{
    string jsFile = outputDir + "/js/name2prot.js";
    OUTJSALL.open(jsFile.c_str(), ios::out);
    if(!OUTJSALL)
    {
        PRINTFUNCTION("Can not open file %s!", jsFile.c_str());
        return false;
    }
    OUTJSALL << "var sourceProteinId=\"" << root << "\";\n";
    OUTJSALL << "var sourceProteinName=\"" << prot2real[root] << "\";\n";
    string example = hasPath[hasPath.size() / 2];
    if((hasPath.size() >= 2) && (hasPath.size() <= 1155))
    {
        example = hasPath[hasPath.size() - 1];
    }
    if(targetFile != "")  //2012-12-04
    {
        for(int i = 1; i < (int)hasPath.size(); i++)
        {
            if(targets.count(hasPath[i]) != 0)
            {
                example = hasPath[i];
                break;
            }
        }
    }
    OUTJSALL << "var exampleProteinId=\"" << example << "\";\n";
    OUTJSALL << "var exampleProteinName=\"" << prot2real[example] << "\";\n";
    OUTJSALL << "var allPaths=new Array();\n";
    OUTJSALL << "var allGraphs=new Array();\n";
    OUTJSALL << "var name2prot = {";
    HASHMAP<string, string, str_hash1>::iterator iter;
    HASHMAP<string, string, str_hash1>::iterator iter_begin = name2prot.begin();
    HASHMAP<string, string, str_hash1>::iterator iter_end = name2prot.end();
    for(iter = iter_begin; iter != iter_end; iter++)
    {
        if(iter != iter_begin)
        {
            OUTJSALL << ", ";
        }
        OUTJSALL << "\"" << iter->first << "\":\"" << iter->second << "\"";
    }
    OUTJSALL << "};\n";
    OUTJSALL << "var id2Path={";
    vector<string> validInputs;
    for(int i = 0; i < (int)hasPath.size(); i++)
    {
        if((targetFile != "") && (targets.count(hasPath[i]) == 0))
        {
            continue; //2012-12-04
        }
        if(i != 0)
        {
            OUTJSALL << ", ";
        }
        ///////////////////////////////////
        string pName = hasPath[i];
        if(prot2real.count(pName) != 0)
        {
            pName = prot2real[pName];
        }
        pName = pName + "," + hasPath[i];
        validInputs.push_back(pName);
        ///////////////////////////////////
        OUTJSALL << "\"" << hasPath[i] << "\":1";
        onePath.clear();
        pathNum = 0;
        outputPath = 1;
        string jsFile = outputDir + "/js/" + hasPath[i] + "_path.js";
        OUTJS1.open(jsFile.c_str(), ios::out);
        if(!OUTJS1)
        {
            PRINTFUNCTION("Can not open file %s!\n", jsFile.c_str());
            return false;
        }
        jsFile = outputDir + "/js/" + hasPath[i] + "_graph.js";
        OUTJS2.open(jsFile.c_str(), ios::out);
        if(!OUTJS2)
        {
            PRINTFUNCTION("Can not open file %s!\n", jsFile.c_str());
            return false;
        }
        OUTJS1 << "allPaths=new Array();\n";
        OUTJS2 << "allGraphs=new Array();\n";
        getPath(hasPath[i]);
        OUTJS1.close();
        OUTJS2.close();
        if(count2 % 10 == 0)
        {
#ifdef INDEP_PROGRAM
            cerr << "\routput: " << count2 << flush;
#else
            PRINTFUNCTION("\rOutput: %d", count2);
            R_FlushConsole();
#endif
        }
        count2++;
    }
    OUTJSALL << "};\n";
    /////////////////////////////////////////////////2013-01-24
    OUTJSALL << "var id2ProteinName=[";
    for(int i = 0; i < (int)OutPutID2names.size(); i++)
    {
        if(i != 0)
        {
            OUTJSALL << ",";
        }
        OUTJSALL << "\"" << OutPutID2names[i] << "\"";
    }
    OUTJSALL << "];\n";
    /////////////////////////////////////////////////2013-01-24
    PRINTFUNCTION("\rOutput: %d\n", count2);
    count2 = 0;
    OUTJSALL.close();
#ifdef INDEP_PROGRAM
#else
    R_FlushConsole();
#endif
    //////////////////////////////////////////////2013-01-05
    string validIuputFile = outputDir + "/validInputProteins.txt";
    ofstream VALIDOUT(validIuputFile.c_str());
    if(!VALIDOUT)
    {
        PRINTFUNCTION("Can not open file %s!", validIuputFile.c_str());
        return false;
    }
    sort(validInputs.begin(), validInputs.end());
    for(int i = 0; i < (int)validInputs.size(); i++)
    {
        VALIDOUT << validInputs[i] << "\n";
    }
    VALIDOUT.close();
    //////////////////////////////////////////////
    return true;
}

bool lessFunction(const string &m1, const string &m2);
bool detectPath(string root)
{
    list<Node> u;
    int targetProteins = 0;
    targets[root] = 1;
    for(int i = 0; i < (int)nodes.size(); i++)
    {
        dist[nodes[i]] = INF;
        prev[nodes[i]] = nodes[i];
        Node tmp;
        tmp.name = nodes[i];
        tmp.dist = INF;
        if(nodes[i] == root)
        {
            tmp.dist = 0;
            dist[nodes[i]] = 0;
            continue;
        }
        u.push_back(tmp);
    }
    Node tmp;
    tmp.name = root;
    tmp.dist = 0;
    u.push_back(tmp);
#ifdef INDEP_PROGRAM
    cerr << "Searching ..." << endl;
    cerr << flush;
#else
    PRINTFUNCTION("Searching ...\n");
    R_FlushConsole();
#endif
    vector<string> s;
    int ccc = u.size();
    PRINTFUNCTION("Number of proteins:%d\n", ccc);
    while(u.size() > 0)
    {
        int ccc = s.size();
        if(ccc % 1000 == 0)
        {
#ifdef INDEP_PROGRAM
            cerr << "\rNumber of processed proteins:" << ccc << flush;
#else
            PRINTFUNCTION("\rNumber of processed proteins:%d", ccc);
            R_FlushConsole();
#endif
        }
        //make_heap(u.begin(), u.end());
        //sort_heap(u.begin(), u.end());
        //sort(u.begin(), u.end());
        string n = u.back().name;
        if(dist[n] >= (INF))
        {
            break;
        }
        u.pop_back();
        s.push_back(n);
        ////////////////////////////////////2012-12-04
        if(targets.size() > 1)
        {
            if(targets.count(n) != 0)
            {
                targetProteins++;
                if(targetProteins == (int)targets.size())
                {
                    break;
                }
            }
        }
        ////////////////////////////////////
        map<string, int>::iterator iter;
        map<string, int>::iterator iter_end = edge[n].end();
        for(iter = edge[n].begin(); iter != iter_end; iter++)
        {
            string protein = iter->first;
            int dis = iter->second;
            if(dist[protein] == dist[n] + dis)
            {
                dist[protein] = dist[n] + dis;
                prev[protein] += "#" + n;
            }
            if(dist[protein] > dist[n] + dis)
            {
                dist[protein] = dist[n] + dis;
                prev[protein] = n;
            }
        }
        list<Node>::iterator it;
        for(it = u.begin(); it != u.end();)
        {
            if(it->dist == dist[it->name])
            {
                it++;
                continue;
            }
            it->dist = dist[it->name];
            list<Node>::iterator it2 = it;
            it2++;
            if(it2 == u.end())
            {
                it++;
                continue;
            }
            while(it2 != u.end())
            {
                if(*it2 < *it)
                {
                    it2++;
                }
                else
                {
                    break;
                }
            }
            Node tmp;
            tmp.name = it->name;
            tmp.dist = it->dist;
            u.insert(it2, tmp);
            it = u.erase(it);
        }
    }
    ccc = s.size();
    hasPath = s;
    PRINTFUNCTION("\rNumber of processed proteins:%d\n", ccc);
#ifdef INDEP_PROGRAM
#else
    R_FlushConsole();
#endif
    return true;
}
////////////////////////
bool processInput(const char *input)
{
    ifstream in(input);
    if(!in)
    {
        PRINTFUNCTION("Can not open %s\n", input);
        return false;
    }
    char buffer[1000000 + 1];
    bool pnasFormat = 1;
    if(!in.eof())
    {
        in.getline(buffer, 1000000);
        string tmp = buffer;
        trim(tmp);
        if(tmp[tmp.size() - 1] == '\r')
        {
            buffer[tmp.size() - 1] = '\0';
            tmp = buffer;
        }
        vector<string> tokens = string_tokenize(tmp, "\t", false);

        if(tokens.size() == 20)
        {
            PRINTFUNCTION("20 columns: standard MITAB format from PINA\n");
            withPubmed = true;
        }
        else if(tokens.size() == 6)
        {
            PRINTFUNCTION("6 columns: valid header\n");
            if(tokens[5] == "PubmedID")
            {
                withPubmed = true;
            }
            pnasFormat = 0;
        }
        else
        {
            PRINTFUNCTION("Invalid file format!\n");
            return false;
        }
    }
    int lineCount = 1;
    while(!in.eof())
    {
        in.getline(buffer, 1000000);
        //PRINTFUNCTION("\rProcessed %d", lineCount);
        lineCount++;
        string tmp = buffer;
        trim(tmp);
        if(tmp[tmp.size() - 1] == '\r')
        {
            buffer[tmp.size() - 1] = '\0';
            tmp = buffer;
        }
        vector<string> tokens = string_tokenize(tmp, "\t", false);
        if((pnasFormat == 1) && (tokens.size() == 20))
        {
            if(tokens[0].substr(0, 10) == "uniprotkb:")
            {
                tokens[0] = tokens[0].substr(10);
            }
            if(tokens[1].substr(0, 10) == "uniprotkb:")
            {
                tokens[1] = tokens[1].substr(10);
            }
            if(tokens[2].substr(0, 10) == "uniprotkb:")
            {
                tokens[2] = tokens[2].substr(10);
                string::size_type pos = tokens[2].find_first_of("(", 0);
                if(pos != string::npos)
                {
                    if(pos == 0)
                    {
                        tokens[2] = tokens[0];
                    }
                    else
                    {
                        tokens[2] = tokens[2].substr(0, pos);
                    }
                }
            }
            if(tokens[3].substr(0, 10) == "uniprotkb:")
            {
                tokens[3] = tokens[3].substr(10);
                string::size_type pos = tokens[3].find_first_of("(", 0);
                if(pos != string::npos)
                {
                    if(pos == 0)
                    {
                        tokens[3] = tokens[1];
                    }
                    else
                    {
                        tokens[3] = tokens[3].substr(0, pos);
                    }
                }
            }
            vector<string> pubmedIds = string_tokenize(tokens[8], "\\|");
            string pubmedstr = "";
            map<string, bool> pubmeds;
            for(int index = 0; index < (int)pubmedIds.size(); index++)
            {
                if((pubmedIds[index].substr(0, 7) == "pubmed:") &&
                        (pubmedIds[index].substr(0, 17) != "pubmed:unassigned"))
                {
                    if(pubmedstr == "")
                    {
                        pubmedstr = pubmedIds[index].substr(7);
                        pubmeds[pubmedstr] = true;
                    }
                    else
                    {
                        string id = pubmedIds[index].substr(7);
                        if(pubmeds.find(id) == pubmeds.end())
                        {
                            pubmedstr = pubmedstr + ", " + id;
                            pubmeds[id] = true;
                        }
                    }
                }
            }
            name2prot[tokens[2]] = tokens[0];
            name2prot[tokens[3]] = tokens[1];
            prot2real[tokens[0]] = tokens[2];
            prot2real[tokens[1]] = tokens[3];
            if(edge.count(tokens[0]) == 0)
            {
                map<string, int> tmp;
                edge[tokens[0]] = tmp;
            }
            if(edge.count(tokens[1]) == 0)
            {
                map<string, int> tmp;
                edge[tokens[1]] = tmp;
            }
            edge[tokens[0]][tokens[1]] = 1;
            edge[tokens[1]][tokens[0]] = 1;
            edgeInfo[tokens[0] + "&" + tokens[1]] = pubmedstr;
            edgeInfo[tokens[1] + "&" + tokens[0]] = pubmedstr;
            if(nodesExist.count(tokens[0]) == 0)
            {
                nodes.push_back(tokens[0]);
                nodesExist[tokens[0]] = 1;
            }
            if(nodesExist.count(tokens[1]) == 0)
            {
                nodes.push_back(tokens[1]);
                nodesExist[tokens[1]] = 1;
            }
            //cerr<<tokens[0]<<endl;
            //cerr<<tokens[1]<<endl;
            //cerr<<tokens[2]<<endl;
            //cerr<<tokens[3]<<endl;
            //cerr<<pubmedstr<<endl;
        }
        if((pnasFormat != 1) && (tokens.size() == 6))
        {
            vector<string> tokens = string_tokenize(tmp, "\t", false);
            name2prot[tokens[2]] = tokens[0];
            name2prot[tokens[3]] = tokens[1];
            prot2real[tokens[0]] = tokens[2];
            prot2real[tokens[1]] = tokens[3];
            if(edge.count(tokens[0]) == 0)
            {
                map<string, int> tmp;
                edge[tokens[0]] = tmp;
            }
            if(edge.count(tokens[1]) == 0)
            {
                map<string, int> tmp;
                edge[tokens[1]] = tmp;
            }
            edge[tokens[0]][tokens[1]] = 1;
            edge[tokens[1]][tokens[0]] = 1;
            if((tokens[4] != "NA") && (tokens[4] != ""))
            {
                if((edgeInfo.count(tokens[0] + "&" + tokens[1]) == 0) ||
                        (edgeInfo[tokens[0] + "&" + tokens[1]] == ""))
                {
                    edgeInfo[tokens[0] + "&" + tokens[1]] = tokens[4];
                    edgeInfo[tokens[1] + "&" + tokens[0]] = tokens[4];
                }
                else
                {
                    edgeInfo[tokens[0] + "&" + tokens[1]] += ("#" + tokens[4]);
                    edgeInfo[tokens[1] + "&" + tokens[0]] += ("#" + tokens[4]);
                }
            }
            if((tokens[5] != "NA") && (tokens[5] != ""))
            {
                if((edgeInfo_string.count(tokens[0] + "&" + tokens[1]) == 0) ||
                        (edgeInfo_string[tokens[0] + "&" + tokens[1]] == ""))
                {
                    edgeInfo_string[tokens[0] + "&" + tokens[1]] = tokens[5];
                    edgeInfo_string[tokens[1] + "&" + tokens[0]] = tokens[5];
                }
                else
                {
                    edgeInfo_string[tokens[0] + "&" + tokens[1]] +=
                        ("#" + tokens[5]);
                    edgeInfo_string[tokens[1] + "&" + tokens[0]] +=
                        ("#" + tokens[5]);
                }
            }
            if(edgeInfo.count(tokens[0] + "&" + tokens[1]) == 0)
            {
                edgeInfo[tokens[0] + "&" + tokens[1]] = "";
                edgeInfo[tokens[1] + "&" + tokens[0]] = "";
            }
            if(nodesExist.count(tokens[0]) == 0)
            {
                nodes.push_back(tokens[0]);
                nodesExist[tokens[0]] = 1;
            }
            if(nodesExist.count(tokens[1]) == 0)
            {
                nodes.push_back(tokens[1]);
                nodesExist[tokens[1]] = 1;
            }
        }
    }
    return true;
}
string int2str(int value);
string double2str(double value);

bool addNodeStr(string Name, int group)
{
    if(prot2real.count(Name) != 0)
    {
        //Name = prot2real[Name] + ":" + Name;
        Name = prot2real[Name];
    }
    else
    {
        //Name = Name + ":" + Name;
        Name = Name;
    }
    int outputID = 0;
    if(name2OutPutID.count(Name) != 0)
    {
        outputID = name2OutPutID[Name];
    }
    else
    {
        outputID = (int)(OutPutID2names.size());
        OutPutID2names.push_back(Name);
        name2OutPutID[Name] = outputID;
    }
    if(nodeStr == "")
    {
        nodeStr = int2str(outputID) + ";" + int2str(group);
    }
    else
    {
        nodeStr += "&";
        nodeStr += int2str(outputID) + ";" + int2str(group);
    }
    return true;
}

bool addNode(string Name, int group)
{
    if(nodeName2id.count(Name) == 0)
    {
        nodeName2id[Name] = id;
        id++;
        if(group == -1)
        {
            group = nodeName2id[Name];
        }
        else
        {
            //childs.push_back(Name);
        }
        addNodeStr(Name, group);
        return true;
    }
    return false;
}
string processEvidence(string evidence)
{
    vector<string> pubmedIds = string_tokenize(evidence, "#");
    string pubmedstr = "";
    map<string, bool> pubmeds;
    for(int index = 0; index < (int)pubmedIds.size(); index++)
    {
        if(pubmedstr == "")
        {
            pubmedstr = pubmedIds[index];
            pubmeds[pubmedstr] = true;
        }
        else
        {
            string id = pubmedIds[index];
            if(pubmeds.find(id) == pubmeds.end())
            {
                pubmedstr = pubmedstr + ", " + id;
                pubmeds[id] = true;
            }
        }
    }
    return pubmedstr;
}
bool addLink(string source, string target, int group, int bigNode)
{
    if(edge.count(source) == 0)
    {
        PRINTFUNCTION("Thers is something wrong! Position1\n");
        return false;
    }
    if(edge[source].count(target) == 0)
    {
        PRINTFUNCTION("Thers is something wrong! Position2\n");
        return false;
    }
    int value = 500;
    string value2 = processEvidence(edgeInfo[source + "&" + target]);
    string value3 = processEvidence(edgeInfo_string[source + "&" + target]);
    if(link2id.count(source + "&" + target) > 0)
    {
        return false;
    }
    if(link2id.count(target + "&" + source) > 0)
    {
        return false;
    }
    link2id[target + "&" + source] = 1;
    if(bigNode > 0)
    {
        value += 1000;
    }
    value = value * 10 / 1000;
    if(nodeName2id.count(source) == 0)
    {
        addNode(source, group);
    }
    if(nodeName2id.count(target) == 0)
    {
        addNode(target, group);
    }
    source = int2str(nodeName2id[source]);
    target = int2str(nodeName2id[target]);
    if(linkStr == "")
    {
        if(bigNode > 0)
        {
           linkStr=source+";"+target+";"+int2str(value)+";"+value2+";"+value3;
        }
        else
        {
            linkStr = source + ";" + target + ";" + int2str(value);
        }
    }
    else
    {
        linkStr += "&";
        if(bigNode > 0)
        {
           linkStr+=source+";"+target+";"+int2str(value)+";"+value2+";"+value3;
        }
        else
        {
            linkStr += source + ";" + target + ";" + int2str(value);
        }
    }
    return true;
}
string currentNode;
bool lessFunction2(const string &m1, const string &m2)
{
    int value1 = edge[currentNode][m1];
    int value2 = edge[currentNode][m2];
    if(value1 != value2)
    {
        return value1 < value2;
    }
    if(withPubmed == true)
    {
        int size1 =
            string_tokenize(edgeInfo[currentNode + "&" + m1], ", ").size();
        int size2 =
            string_tokenize(edgeInfo[currentNode + "&" + m2], ", ").size();
        if(edgeInfo[currentNode + "&" + m1] == "")
        {
            size1 = 0;
        }
        if(edgeInfo[currentNode + "&" + m2] == "")
        {
            size2 = 0;
        }
        if(size1 != size2)
        {
            return size1 > size2;
        }
    }
    if(m1 != m2)
    {
        return m1 < m2;
    }
    return false;
}
bool addLink2(string name)
{
    mainNodes.push_back(name);
    map<string, int>::iterator iter;
    for(iter = edge[name].begin(); iter != edge[name].end(); iter++)
    {
        childs.push_back(iter->first);
    }
    return true;
}
bool addChildLink(string source, string target)
{
    if(edge.count(source) == 0)
    {
        PRINTFUNCTION("Thers is something wrong! Position1\n");
        return false;
    }
    if(edge[source].count(target) == 0)
    {
        //PRINTFUNCTION("Thers is something wrong! Position2\n");
        return false;
    }
    int value = 500;
    string value2 = edgeInfo[source + "&" + target];
    if(link2id.count(source + "&" + target) > 0)
    {
        return false;
    }
    if(link2id.count(target + "&" + source) > 0)
    {
        return false;
    }
    link2id[target + "&" + source] = 1;
    value = value * 10 / 1000;
    if(nodeName2id.count(source) == 0)
    {
        PRINTFUNCTION("Thers is something wrong! Position3\n");
    }
    if(nodeName2id.count(target) == 0)
    {
        PRINTFUNCTION("Thers is something wrong! Position4\n");
    }
    source = int2str(nodeName2id[source]);
    target = int2str(nodeName2id[target]);
    if(linkStr == "")
    {
        linkStr = string("{\"source\":") + source + ", \"target\":" +
                  target + ", \"value\":" + int2str(value) + "\"}";
    }
    else
    {
        linkStr += ", ";
        linkStr += string("{\"source\":") + source + ", \"target\":" +
                   target + ", \"value\":" + int2str(value) + "\"}";
    }
    return true;

}
int changeint(int value)
{
    if(value % 2 == 0)
    {
        return 1;
    }
    if(value % 2 == 1)
    {
        return 7;
    }
    return 1;
}
bool addRootLinks(vector<string> &allChilds, vector<string> &mainNodes)
{
    if(mainNodes.size() != 1)
    {
        return false;
    }
    sort(allChilds.begin(), allChilds.end());
    for(int i = 0; i < (int)allChilds.size(); i++)
    {
        vector<string> valideNodes;
        for(int j = 0; j < (int)mainNodes.size(); j++)
        {
            if(edgeInfo.count(allChilds[i] + "&" + mainNodes[j]) != 0)
            {
                valideNodes.push_back(mainNodes[j]);
            }
        }
        if(valideNodes.size() == 0)
        {
            PRINTFUNCTION("Thers is something wrong! Position10\n");
        }
        if((valideNodes.size() == 1) && (mainNodes.size() == 1))
        {
            for(int j = 0; j < (int)valideNodes.size(); j++)
            {
                addLink(valideNodes[j], allChilds[i], changeint(i) + 1000, -1);
            }
        }
        if(i >= 128)
        {
            break;
        }
    }
    return true;

}
bool addChildLinks(vector<string> &allChilds, vector<string> &mainNodes)
{
    //sort(allChilds.begin(), allChilds.end());
    for(int i = 0; i < (int)allChilds.size(); i++)
    {
        vector<string> valideNodes;
        for(int j = 0; j < (int)mainNodes.size(); j++)
        {
            if(edgeInfo.count(allChilds[i] + "&" + mainNodes[j]) != 0)
            {
                valideNodes.push_back(mainNodes[j]);
            }
        }
        if(valideNodes.size() == 0)
        {
            PRINTFUNCTION("Thers is something wrong! Position10\n");
        }
        if(valideNodes.size() >= 2)
        {
            for(int j = 0; j < (int)valideNodes.size(); j++)
            {
                addLink(valideNodes[j], allChilds[i], 7 + 1000, -1);
            }
        }
    }
    return true;
}
///////////////////////////////////////////////////
bool addInfoFirst()
{
    name2prot.clear();
    if(n2pFile == "")
    {
        return true;
    }
    ifstream in(n2pFile.c_str());
    if(!in)
    {
        PRINTFUNCTION("Can not open %s\n", n2pFile.c_str());
        return false;
    }
    char buffer[1000000 + 1];
    while(!in.eof())
    {
        in.getline(buffer, 1000000);
        string tmp = buffer;
        if(tmp[tmp.size() - 1] == '\r')
        {
            tmp[tmp.size() - 1] = '\0';
        }
        vector<string> tokens = string_tokenize(tmp, "\t", false);
        if(tokens.size() < 2)
        {
            continue;
        }
        if(name2prot.count(tokens[1]) == 0)
        {
            name2prot[tokens[1]] = tokens[0];
        }
    }
    in.close();
    return true;
}

bool getTargets(string outputDir)
{
    targets.clear();
    if(targetFile == "")
    {
        return true;
    }
    ifstream in(targetFile.c_str());
    if(!in)
    {
        PRINTFUNCTION("Can not open %s\n", targetFile.c_str());
        return false;
    }
    ofstream out;
    string output = outputDir + "/targetIDs.txt";
    out.open(output.c_str(), ios::out);
    if(!out)
    {
        PRINTFUNCTION("Can not open file %s!", output.c_str());
        return false;
    }
    char buffer[1000000 + 1];
    while(!in.eof())
    {
        in.getline(buffer, 1000000);
        string tmp = buffer;
        if(tmp.size() <= 2)
        {
            continue;
        }
        if(tmp[tmp.size() - 1] == '\r')
        {
            buffer[tmp.size() - 1] = '\0';
            tmp = buffer;
        }
        vector<string> tokens = string_tokenize(tmp, ",", false);
        if(tokens.size() < 1)
        {
            out << tmp << "#" << "invalid format\n";
            continue;
        }
        if(tokens.size() == 1)
        {
            tokens.push_back("");
        }
        if(tokens[1] != "")
        {
            out << tmp << "#" << "valid ID: " << tokens[1] << "\n";
            targets[tokens[1]] = 1;
        }
        else
        {
            if(name2prot.count(tokens[0]) == 0)
            {
                out << tmp << "#" << "invalid protein name\n";
            }
            else
            {
                out <<tmp<<"#"<<"valid ID: "<<name2prot[tokens[0]]<<"\n";
                targets[name2prot[tokens[0]]] = 1;
            }
        }
    }
    in.close();
    out.close();
    if(targets.size() == 0)
    {
        return false;
    }
    return true;
}
///////////////////////////////////////////////////
inline vector<string> string_tokenize(const string &str,
                                      const string &delimiters,
                                      bool skip_empty)
{
    // Skip delimiters at beginning.
    string::size_type lastPos = skip_empty ?
                                str.find_first_not_of(delimiters, 0) : 0;
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);
    vector<string> result;
    result.clear();

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        //if (pos == lastPos) result.push_back("");
        result.push_back(str.substr(lastPos, pos - lastPos));

        if (pos == string::npos) break;
        if (pos == str.size() - 1)
        {
            if (!skip_empty) result.push_back("");
            break;
        }
        // Skip delimiters.  Note the "not_of"
        lastPos = skip_empty ? str.find_first_not_of(delimiters, pos) : pos + 1;
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
    return result;
}
string int2str(int value)
{
    char tmp[20];
    sprintf(tmp, "%d", value);
    return tmp;
}
string double2str(double value)
{
    char tmp[20];
    sprintf(tmp, "%.6f", value);
    return tmp;
}
void ToUpperString(string &str)
{
    transform(str.begin(), str.end(), str.begin(), (int ( *)(int))toupper);
}
string trim(string &s)
{
    if (s.size() == 0) return s;
    size_t beg = s.find_first_not_of(" \a\b\f\n\r\t\v");
    size_t end = s.find_last_not_of(" \a\b\f\n\r\t\v");
    if (beg == string::npos) return "";
    return string(s, beg, end - beg + 1);
}
bool lessFunction(const string &m1, const string &m2)
{
    int value1 = dist[m1];
    int value2 = dist[m2];
    if(value1 == value2)
    {
        return m1 > m2;
    }
    else
    {
        return (value1 > value2);
    }
}
/*struct str_hash1{
    size_t operator()(const string& str) const{
           unsigned long __h = 0;
           for (size_t i = 0 ; i < str.size() ; i ++){
               __h = 107*__h + str[i];
           }
           return size_t(__h);
    }
};*/

/*struct str_hash{
    size_t operator()(const string& str) const{
           return __stl_hash_string(str.c_str());
    }
};*/
struct str_equal
{
    bool operator()(const string &s1, const string &s2) const
    {
        return s1 == s2;
    }
};
string get_package_bin_path(string path)
{
    string::size_type lastslash = path.rfind('/');
    if(lastslash == string::npos)
    {
        lastslash = path.rfind('\\');
    }
    if(lastslash == string::npos)
    {
        return "";
    }
    return path.substr(0, lastslash + 1);
}
bool cp_file(string input, string output)
{
    ifstream IN(input.c_str());
    if(!IN)
    {
        PRINTFUNCTION("Can not open file %s\n", input.c_str());
        return false;
    }
    ofstream OUT(output.c_str());
    if(!OUT)
    {
        PRINTFUNCTION("Can not open file %s!", output.c_str());
        return false;
    }
    char buffer[100000 + 1];
    while(!IN.eof())
    {
        IN.getline(buffer, 100000);
        string tmp = buffer;
        OUT << buffer << endl;
    }
    IN.close();
    OUT.close();
    return true;
}
bool cp_html(string path, string outputDir)
{
    cp_file(path + "template/network.html", outputDir + "/network.html");
    cp_file(path + "template/d3.js", outputDir + "/D3/d3.js");
    cp_file(path + "template/d3.geom.js", outputDir + "/D3/d3.geom.js");
    cp_file(path + "template/d3.layout.js", outputDir + "/D3/d3.layout.js");
    return true;
}
bool makedir(const char *dir, int mask)
{
    string system_cmd("mkdir ");
#ifdef WINDOWS
    system_cmd += dir;
    std::system(system_cmd.c_str());
    return true;
#else
    system_cmd = "mkdir -p ";
    system_cmd += dir;
    std::system(system_cmd.c_str());
    return true;
#endif
}
////////////////////////////////////////////////////////////////////
vector <string> proteinNodes;
HASHMAP<string, int, str_hash1> name2big;
HASHMAP<string, string, str_hash1> name2color;
int colNodeid = 0;
bool clearInfo()
{
    name2prot.clear();
    prot2real.clear();
    edge.clear();
    edgeInfo.clear();
    edgeInfo_string.clear();
    nodes.clear();
    nodesExist.clear();
    dist.clear();
    prev.clear();
    hasPath.clear();
    withPubmed = false;
    nodeName2id.clear();
    link2id.clear();
    childs.clear();
    mainNodes.clear();
    onePath.clear();
    n2pFile = "";
    targetFile = "";
    targets.clear();
    proteinNodes.clear();
    name2big.clear();
    name2color.clear();
    colNodeid = 0;
    outputID = 0;
    OutPutID2names.clear();
    name2OutPutID.clear();
    return true;
}
bool addColorNodeStr(string Name)
{
    if(nodeName2id.count(Name) != 0)
    {
        return false;
    }
    nodeName2id[Name] = colNodeid;
    colNodeid++;
    int big = name2big[Name];
    string color = "'" + name2color[Name] + "'";
    if(prot2real.count(Name) != 0)
    {
        Name = prot2real[Name] + ":" + Name;
    }
    else
    {
        Name = Name + ":" + Name;
    }
    if(nodeStr == "")
    {
        nodeStr = string("{\"name\":") + "\"" + Name + "\"" + "," + "\"group\":"
                  + int2str(big) + "," + "\"color\":" + color + "}";
    }
    else
    {
        nodeStr += ", ";
        nodeStr+=string("{\"name\":") + "\"" + Name + "\"" + "," + "\"group\":"
                   + int2str(big) + "," + "\"color\":" + color + "}";
    }
    return true;
}
bool addColorLink(string source, string target)
{
    if(edge.count(source) == 0)
    {
        return false;
    }
    if(edge[source].count(target) == 0)
    {
        return false;
    }
    int value = 500;
    string value2 = processEvidence(edgeInfo[source + "&" + target]);
    string value3 = processEvidence(edgeInfo_string[source + "&" + target]);
    if(link2id.count(source + "&" + target) > 0)
    {
        return false;
    }
    if(link2id.count(target + "&" + source) > 0)
    {
        return false;
    }
    link2id[target + "&" + source] = 1;
    if((name2big[source] == 1) && (name2big[target] == 1))
    {
        value += 1000;
    }
    value = value * 10 / 1000;
    source = int2str(nodeName2id[source]);
    target = int2str(nodeName2id[target]);
    if(linkStr == "")
    {
        linkStr = string("{\"source\":") + source + ", \"target\":" + target
                  + ", \"value\":" + int2str(value) + ", \"pubmed\":\""
                  + value2 + "\", \"Evidence\":\"" + value3 + "\"}";
    }
    else
    {
        linkStr += ", ";
        linkStr += string("{\"source\":") + source + ", \"target\":" + target
                   + ", \"value\":" + int2str(value) + ", \"pubmed\":\""
                   + value2 + "\", \"Evidence\":\"" + value3 + "\"}";
    }
    return true;
}
bool addColorChilds(vector<string> &allChilds,
                    vector<string> &mainNodes,
                    string col)
{
    //sort(allChilds.begin(), allChilds.end());
    for(int i = 0; i < (int)allChilds.size(); i++)
    {
        string aChild = allChilds[i];
        vector<string> valideNodes;
        for(int j = 0; j < (int)mainNodes.size(); j++)
        {
            if(edgeInfo.count(allChilds[i] + "&" + mainNodes[j]) != 0)
            {
                valideNodes.push_back(mainNodes[j]);
            }
        }
        if(valideNodes.size() == 0)
        {
            PRINTFUNCTION("Thers is something wrong! Position10\n");
        }
        if(valideNodes.size() >= 2)
        {
            for(int j = 0; j < (int)valideNodes.size(); j++)
            {
                if(name2color.count(aChild) == 0)
                {
                    name2color[aChild] = col;
                }
                else
                {
                    //PRINTFUNCTION("Thers is something wrong! Position11\n");
                }
                if(name2big.count(aChild) == 0)
                {
                    name2big[aChild] = 0;
                }
                else
                {
                    //PRINTFUNCTION("Thers is something wrong! Position12\n");
                }
                addColorNodeStr(aChild);
                addColorLink(valideNodes[j], aChild);
            }
        }
    }
    return true;
}
bool getProteinNodes(string proteinFile, string outputDir)
{
    ifstream in(proteinFile.c_str());
    if(!in)
    {
        PRINTFUNCTION("Can not open %s\n", proteinFile.c_str());
        return false;
    }
    ofstream out;
    string output = outputDir + "/proteinIDs.txt";
    out.open(output.c_str(), ios::out);
    if(!out)
    {
        PRINTFUNCTION("Can not open file %s!", output.c_str());
        return false;
    }
    char buffer[10000 + 1];
    while(!in.eof())
    {
        in.getline(buffer, 10000);
        string tmp = buffer;
        if(tmp.size() <= 2)
        {
            continue;
        }
        if(tmp[tmp.size() - 1] == '\r')
        {
            buffer[tmp.size() - 1] = '\0';
            tmp = buffer;
        }
        vector<string> tokens = string_tokenize(tmp, ",", false);
        if(tokens.size() != 4)
        {
            out << tmp << "#" << "invalid format\n";
            continue;
        }
        string swissID = "";
        if(tokens[1] == "1")
        {
            proteinNodes.push_back(tokens[0]);
            swissID = tokens[0];
        }
        else
        {
            if(name2prot.count(tokens[0]) == 0)
            {
                out << tmp << "#" << "invalid protein name\n";
                continue;
            }
            else
            {
                out<<tmp<<"#"<<"valid ID: "<<name2prot[tokens[0]]<< "\n";
                proteinNodes.push_back(name2prot[tokens[0]]);
                swissID = name2prot[tokens[0]];
            }
        }
        if(tokens[2] == "1")
        {
            name2big[swissID] = 1;
        }
        else
        {
            name2big[swissID] = 0;
        }
        name2color[swissID] = tokens[3];
    }
    in.close();
    out.close();
    return false;
}
bool viewGraph(const char *input, const char *proteinFile,
               const char *output, int addChilds, const char *childCol)
{
    PRINTFUNCTION("Processing input file\n");
    PRINTFUNCTION("input file: %s\n", input);
    PRINTFUNCTION("protein file: %s\n", proteinFile);
    PRINTFUNCTION("output directory: %s\n", output);
    string outputFile = string(output) + "/js/graph.js";
#ifdef INDEP_PROGRAM
#else
    R_FlushConsole();
#endif
    //return true;
    ofstream OUT(outputFile.c_str());
    if(!OUT)
    {
        PRINTFUNCTION("Can not open file:%s\n", outputFile.c_str());
        return false;
    }
    if(!processInput(input))
    {
        return false;
    }
    getProteinNodes(proteinFile, output);
    nodeName2id.clear();
    link2id.clear();
    nodeStr = "";
    linkStr = "";
    vector<string> colorChilds;
    for(int i = 0; i < (int)proteinNodes.size(); i++)
    {
        addColorNodeStr(proteinNodes[i]);
        string name = proteinNodes[i];
        map<string, int>::iterator iter;
        for(iter = edge[name].begin(); iter != edge[name].end(); iter++)
        {
            colorChilds.push_back(iter->first);
        }
    }
    for(int i = 0; i < (int)proteinNodes.size(); i++)
    {
        for(int j = 0; j < (int)proteinNodes.size(); j++)
        {
            string source = proteinNodes[i];
            string target = proteinNodes[j];
            if(name2big[source] == 1)
            {
                addColorLink(source, target);
                continue;
            }
            if(name2big[target] == 1)
            {
                addColorLink(target, source);
                continue;
            }
            addColorLink(source, target);
        }
    }
    if(addChilds == 1)
    {
        addColorChilds(colorChilds, proteinNodes, childCol);
    }
    OUT << "graphView=" << "{\"nodes\":[" + nodeStr
        + "], \"links\":[" + linkStr + "]};\n";
    OUT.close();
    return true;
}
//////////////////////////////////////////////////////////////////////////////
bool addInfo(string infoFileName, string resultDir)
{
    HASHMAP<string, string, str_hash1> name2prot;
    resultDir = resultDir + "/js/name2prot.js";
    name2prot.clear();
    ifstream in(resultDir.c_str());
    if(!in)
    {
        PRINTFUNCTION("Can not open %s\n", resultDir.c_str());
        return false;
    }
    ifstream in2(infoFileName.c_str());
    if(!in2)
    {
        PRINTFUNCTION("Can not open %s\n", infoFileName.c_str());
        return false;
    }
    vector<string> lines;
    string lastLine;
    char buffer[1000000 + 1];
    while(!in.eof())
    {
        in.getline(buffer, 1000000);
        string tmp = buffer;
        if(tmp[tmp.size() - 1] == '\r')
        {
            tmp[tmp.size() - 1] = '\0';
        }
        if(tmp.size() <= 2)
        {
            continue;
        }
        if(tmp.substr(0, 12) == string("var id2Path="))
        {
            lastLine = tmp;
            continue;
        }
        if(tmp.substr(0, 15) != string("var name2prot ="))
        {
            lines.push_back(tmp);
            continue;
        }
        tmp = tmp.substr(16);
        vector<string> tokens = string_tokenize(tmp, ",", false);
        for(int i = 0; i < (int)tokens.size(); i++)
        {
            if((tokens[i][0] == ' ') || (tokens[i][0] == '{'))
            {
                tokens[i] = tokens[i].substr(1);
            }
            if(i == (int)(tokens.size() - 1))
            {
                tokens[i] = tokens[i].substr(0, tokens[i].size() - 2);
            }
            vector<string> smalltokens = string_tokenize(tokens[i], ":", false);
            string name = smalltokens[0];
            string prot = smalltokens[smalltokens.size() - 1];
            for(int i = 1; i < (int)smalltokens.size() - 1; i++)
            {
                name += (":" + smalltokens[i]);
            }
            name2prot[name] = prot;
        }
    }
    in.close();
    while(!in2.eof())
    {
        in2.getline(buffer, 1000000);
        string tmp = buffer;
        if(tmp[tmp.size() - 1] == '\r')
        {
            tmp[tmp.size() - 1] = '\0';
        }
        vector<string> tokens = string_tokenize(tmp, "\t", false);
        if(tokens.size() < 2)
        {
            continue;
        }
        tokens[0] = "\"" + tokens[0] + "\"";
        tokens[1] = "\"" + tokens[1] + "\"";
        if(name2prot.count(tokens[1]) == 0)
        {
            name2prot[tokens[1]] = tokens[0];
        }
    }
    in2.close();
    ofstream OUTJSALL(resultDir.c_str());
    if(!OUTJSALL)
    {
        PRINTFUNCTION("Can not open %s to write\n", resultDir.c_str());
        return false;
    }
    HASHMAP<string, string, str_hash1>::iterator iter;
    HASHMAP<string, string, str_hash1>::iterator iter_begin = name2prot.begin();
    HASHMAP<string, string, str_hash1>::iterator iter_end = name2prot.end();
    for(int i = 0; i < (int)lines.size(); i++)
    {
        OUTJSALL << lines[i] << "\n";
    }
    OUTJSALL << "var name2prot = {";
    for(iter = iter_begin; iter != iter_end; iter++)
    {
        if(iter != iter_begin)
        {
            OUTJSALL << ", ";
        }
        OUTJSALL << iter->first << ":" << iter->second;
    }
    OUTJSALL << "};\n";
    OUTJSALL << lastLine << "\n";
    OUTJSALL.close();
    return true;
}
#ifdef INDEP_PROGRAM
#else
extern "C" {
    int cisPathC(char **input, char **protein, char **output,
                 char **targetsFile, char **name2protFile, int *maxPathCount)
    {
        n2pFile = name2protFile[0];
        targetFile = targetsFile[0];
        maxNum = maxPathCount[0];
        cispath(input[0], protein[0], output[0]);
        /////////////////////////////
        clearInfo();
        return 1;
    }
    int addInfoC(char **infoFileName, char **resultDir)
    {
        //PRINTFUNCTION("infoFileName: %s\n", infoFileName[0]);
        //PRINTFUNCTION("resultDir: %s\n", resultDir[0]);
        addInfo(infoFileName[0], resultDir[0]);
        return 1;
    }
    int viewGraphC(char **input, char **proteinFile,
                   char **outputDir, int *addChilds, char **childCol)
    {
        viewGraph(input[0], proteinFile[0],
                  outputDir[0], addChilds[0], childCol[0]);
        clearInfo();
        return 1;
    }
}

extern "C" {
    static R_NativePrimitiveArgType cisPathC_t[] =
    {STRSXP, STRSXP, STRSXP, STRSXP, STRSXP, INTSXP};
    static R_NativePrimitiveArgType addInfoC_t[] = {STRSXP, STRSXP};
    static R_NativePrimitiveArgType viewGraphC_t[] =
    {STRSXP, STRSXP, STRSXP, INTSXP, STRSXP};
    R_CMethodDef cMethods[] =
    {
        {".cisPathC", (DL_FUNC) &cisPathC, 6, cisPathC_t},
        {".addInfoC", (DL_FUNC) &addInfoC, 2, addInfoC_t},
        {".viewGraphC", (DL_FUNC) &viewGraphC, 5, viewGraphC_t},
        {NULL, NULL, 0}
    };

    void R_init_cisPath(DllInfo *info)
    {
        R_registerRoutines(info, cMethods, NULL, NULL, NULL);
    }
}
#endif