#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
using namespace std;

bool removable(vector<int> neighbor, vector<int> cover);
int max_removable(vector<vector<int> > neighbors, vector<int> cover);
vector<int> procedure_1(vector<vector<int> > neighbors, vector<int> cover);
vector<int> procedure_2(vector<vector<int> > neighbors, vector<int> cover, int k);
int cover_size(vector<int> cover);
ofstream saida ("resultados.txt");
ifstream entrada ("entrada_nsat.txt");

int main() {

    /* INICIO DA ETAPA DE LEITURA E CONVERSÃO DO ARQUIVO EM INSTÂNCIA DE VETOR DE VETORES */
    int nVariaveis = 0, nClausulas = 0, nVariaveisPorClausula = -1;
    string primeiraLinha = "";
    getline(entrada, primeiraLinha);
    stringstream ssPrimeiraLinha (primeiraLinha);
    ssPrimeiraLinha >> nVariaveis;
    ssPrimeiraLinha >> nClausulas;
    if (!ssPrimeiraLinha.eof()) {
        ssPrimeiraLinha >> nVariaveisPorClausula;
    }

    vector< vector<int> > arquivo;
    for (int i = 0; i < nClausulas; i++) {
        string linhaAtual = "";
        getline(entrada, linhaAtual);
        stringstream ssLinhaAtual (linhaAtual);
        vector<int> vetorLinhaAtual;
        while(!ssLinhaAtual.eof()) {
            int numeroAtual = 0;
            ssLinhaAtual >> numeroAtual;
            vetorLinhaAtual.push_back(numeroAtual);
        }
        if (vetorLinhaAtual.size() > 0) {
            arquivo.push_back(vetorLinhaAtual);
        }
    }

    /* FIM DA ETAPA DE LEITURA E CONVERSÃO DO ARQUIVO EM INSTÂNCIA DE VETOR DE VETORES */


    /* INICIO DA ETAPA DE CONVERSÃO DA INSTANCIA DE VETOR DE VETORES DO ARQUIVO
       PARA MATRIZ DE ADJACENCIAS DO GRAFO GERADO */

    int tam = nVariaveis*2 + nClausulas*3;
    vector<vector<int> > grafo;
    for(int i = 0; i < tam; i++){
        vector<int> linhaGrafo;
        for(int j = 0; j < tam; j++){
            linhaGrafo.push_back(0);
        }
        grafo.push_back(linhaGrafo);
    }

    /* Seta arestas entre Xi e -Xi. (subgrafos de 2 vertices)
     * Da posicao 0 até a posicao nVars-1 sao as variaveis normais.
     * Da posicao nVars até nVars*2 - 1 sao as negações das variáveis.
     * Ou seja, pra pegar a linha correspondente à negativa de uma variavel Xi, eu tenho que somar i ao numero
     * de nVars-1 (que eh onde acabam as variaveis positivas).
     */

    for(int i = 0; i < nVariaveis+2; i = i+2){
        //Faz uma aresta entre Xi e -Xi.
        grafo[i][i+1] = 1;
        //Eh preciso setar também na linha da variavel negativa, para fazer a aresta "de volta".
        grafo[i+1][i] = 1;
    }

    /* Agora eh preciso setar os subgrafos de 3 vertices. O indice tem que comecar de nVars*2, pois a ultima variavel
     * negativa vai estar na posicao (nVars*2 - 1).

     * O contador vai parar quando chegar na ultima linha do arquivo. O arquivo tem nClausulas + 1 linhas.
     * O contador ja começa de 1 pois na primeira linha nao ha informacoes sobre clausulas.

     * indice1 ate indice3 sao os indices das variaveis das clausulas
     */
    int indice1 = 0;
    int indice2 = 0;
    int indice3 = 0;

    //indice4 ate indice6 vao guardar os indices das novas linhas 'criadas' pra cada variavel de cada clausula
    int indice4 = 0;
    int indice5 = 0;
    int indice6 = 0;

    //indice_clausula vai ser o indice da nova linha 'criada' pra cada variavel de cada clausula.
    int indice_clausula = nVariaveis*2;

    for(int i = 0 ; i < nClausulas; i++){
        //Verifica se o numero que ele pegou eh negativo. Se for, entao a representacao dele vai ta na linha
        //(nVars - 1) + indice da variavel.
        //Se for positivo, entao a representacao dele vai ta na linha indice - 1.
        indice1 = arquivo[i][0];
        if(indice1 < 0) {
            indice1 = (abs(indice1) - 1)*2 + 1;
        }
        else{
            indice1 = (indice1 - 1)*2;
        }

        //Liga o vértice ao dispositivo de 2 vertices correspondente.
        grafo[indice1][indice_clausula] = 1;
        grafo[indice_clausula][indice1] = 1;
        indice4 = indice_clausula;

        indice_clausula++;

        indice2 = arquivo[i][1];
        if(indice2 < 0) {
            indice2 = (abs(indice2) - 1)*2 + 1;
        }
        else{
            indice2 = (indice2 - 1)*2;
        }

        grafo[indice2][indice_clausula] = 1;
        grafo[indice_clausula][indice2] = 1;
        indice5 = indice_clausula;

        indice_clausula++;

        indice3 = arquivo[i][2];
        if(indice3 < 0) {
            indice3 = (abs(indice3) - 1)*2 + 1;
        }
        else{
            indice3 = (indice3 - 1)*2;
        }

        grafo[indice3][indice_clausula] = 1;
        grafo[indice_clausula][indice3] = 1;
        indice6 = indice_clausula;

        indice_clausula++;

        //Cria arestas entre as 3 variaveis lidas, formando um triangulo. Lembrando que sempre tem que fazer
        //a aresta "indo" e "voltando", pois o grafo é nao orientado.
        grafo[indice4][indice5] = 1;
        grafo[indice4][indice6] = 1;
        grafo[indice5][indice4] = 1;
        grafo[indice5][indice6] = 1;
        grafo[indice6][indice4] = 1;
        grafo[indice6][indice5] = 1;
    }


    /* FIM DA ETAPA DE CONVERSÃO DA INSTANCIA DE VETOR DE VETORES DO ARQUIVO
       PARA MATRIZ DE ADJACENCIAS DO GRAFO GERADO */

    /* INICIO DO ALGORITMO PARA ENCONTRAR MINIMUN VERTEX COVER A PARTIR
       DA MATRIZ DE ADJACENCIAS DO GRAFO GERADO */

    cout<<"Algoritmo 3-SAT por Cobertura de Vertices."<<endl;
    int n, i, j, k, p, q, r, s, min, edge, counter=0;
    n = nVariaveis*2 + nClausulas*3;

    /* Encontrando Vizinhos */
    vector<vector<int> > neighbors;
    for(i=0; i<grafo.size(); i++) {
        vector<int> neighbor;
        for(j=0; j<grafo[i].size(); j++) {
            if(grafo[i][j]==1) {
                neighbor.push_back(j);
            }
        }
        neighbors.push_back(neighbor);
    }
    cout<<"Grafo gerado tem n = "<< n <<" vertices."<<endl;

    /* Tamanho minimo de cobertura de vertices para ser SAT*/
    k = nVariaveis + 2 * nClausulas;

    /* Encontrar cobertura de vertices */
    bool found=false;
    cout<<"Encontrando Cobertura de Vertices..."<<endl;
    min=n+1;
    vector<vector<int> > covers;
    vector<int> allcover;
    for(i=0; i<grafo.size(); i++) {
        allcover.push_back(1);
    }
    for(i=0; i<allcover.size(); i++) {
        if(found) {
            break;
        }
        vector<int> cover=allcover;
        cover[i]=0;
        cover=procedure_1(neighbors,cover);
        s=cover_size(cover);
        if(s<min){
            min=s;
        }
        if(s<k) {
            covers.push_back(cover);
            found=true;
            break;
        }
        for(j=0; j<n-k; j++) {
            cover=procedure_2(neighbors,cover,j);
        }
        s=cover_size(cover);
        if(s<min) {
            min=s;
        }
        covers.push_back(cover);
        if(s<k){
            found=true;
            break;
        }
    }

    /* IMPRIME RESULTADOS NO PROMPT */
    if (found) cout << "Procura encerrada! Encontrada cobertura de vertices de tamanho: "<<min<<"."<<endl;
    else cout << "Tamanho de cobertura de vertices minima encontrada eh: "<<min<<"."<<endl;
    cout << "K = numVariaveis + 2 * numClausulas = " << nVariaveis << " + 2 * " << nClausulas << " = "
    << k << endl;
    cout << "Qualquer cobertura por vertices do grafo gerado deve ter pelo menos k vertices" << endl;
    cout << "Existe uma cobertura de k vertices do grafo gerado se e somente se a instancia do problema" <<
        " original 3-SAT for satisfazivel. Sendo assim:" << endl;
    if (min < k) {
        cout << "O problema nao eh SAT." << endl;
    } else {
        cout << "O problema eh SAT." << endl;
    }
    cout << endl << "Resultados estao registrados no arquivo 'resultados.txt'." << endl;

    /* IMPRIME RESULTADOS NO ARQUIVO .TXT*/
    if (found) saida << "Procura encerrada! Encontrada cobertura de vertices de tamanho: "<<min<<"."<<endl;
    else saida << "Tamanho de cobertura de vertices minima encontrada eh: "<<min<<"."<<endl;
    saida << "K = numVariaveis + 2 * numClausulas = " << nVariaveis << " + 2 * " << nClausulas << " = "
    << nVariaveis+(2*nClausulas) << endl;
    //cout<<"See cover.txt for results."<<endl;
    saida << "Qualquer cobertura por vertices do grafo gerado deve ter pelo menos k vertices" << endl;
    saida << "Existe uma cobertura de k vertices do grafo gerado se e somente se a instancia do problema" <<
        " original 3-SAT for satisfazivel. Sendo assim:" << endl;
    if (min < (nVariaveis+(2*nClausulas))) {
        saida << "O problema nao eh SAT." << endl;
    } else {
        saida << "O problema eh SAT." << endl;
    }

    return 0;
}

bool removable(vector<int> neighbor, vector<int> cover) {
    bool check=true;
    for(int i=0; i<neighbor.size(); i++) {
        if(cover[neighbor[i]]==0) {
            check=false;
            break;
        }
    }
    return check;
}

int max_removable(vector<vector<int> > neighbors, vector<int> cover) {
    int r=-1, max=-1;
    for(int i=0; i<cover.size(); i++) {
        if(cover[i]==1 && removable(neighbors[i],cover)==true) {
            vector<int> temp_cover=cover;
            temp_cover[i]=0;
            int sum=0;
            for(int j=0; j<temp_cover.size(); j++) {
                if(temp_cover[j]==1 && removable(neighbors[j], temp_cover)==true) {
                    sum++;
                }
                if(sum>max) {
                    max=sum;
                    r=i;
                }
            }
        }
    }
    return r;
}

vector<int> procedure_1(vector<vector<int> > neighbors, vector<int> cover) {
    vector<int> temp_cover=cover;
    int r=0;
    while(r!=-1) {
        r= max_removable(neighbors,temp_cover);
        if(r!=-1) {
            temp_cover[r]=0;
        }
    }
    return temp_cover;
}

vector<int> procedure_2(vector<vector<int> > neighbors, vector<int> cover, int k) {
    int count=0;
    vector<int> temp_cover=cover;
    for(int i=0; i<temp_cover.size(); i++) {
        if(temp_cover[i]==1) {
            int sum=0, index;
            for(int j=0; j<neighbors[i].size(); j++) {
                if(temp_cover[neighbors[i][j]]==0) {
                    index=j;
                    sum++;
                }
            }
            if(sum==1 && cover[neighbors[i][index]]==0) {
                temp_cover[neighbors[i][index]]=1;
                temp_cover[i]=0;
                temp_cover=procedure_1(neighbors,temp_cover);
                count++;
            }
            if(count>k) {
                break;
            }
        }
    }
    return temp_cover;
}

int cover_size(vector<int> cover) {
    int count=0;
    for(int i=0; i<cover.size(); i++) {
        if(cover[i]==1) count++;
    }
    return count;
}
