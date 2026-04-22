#include "Tensor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <numeric>

namespace nexa {
Tensor matmul(const Tensor& A, const Tensor& B) {
    int m=A.shape[0],n=A.shape[1],p=B.shape[1];
    std::vector<float> result(m*p,0.f);
    for(int i=0;i<m;i++) for(int j=0;j<p;j++){float s=0;for(int k=0;k<n;k++)s+=A.data[i*n+k]*B.data[k*p+j];result[i*p+j]=s;}
    return Tensor(result,{m,p});
}
} // namespace nexa

static float sigmoid(float x){return 1.f/(1.f+std::exp(-x));}
static std::string trim(const std::string& s){size_t a=s.find_first_not_of(" \t\r\n"),b=s.find_last_not_of(" \t\r\n");return(a==std::string::npos)?"":s.substr(a,b-a+1);}
static std::vector<std::string> splitLine(const std::string& line,char d=','){std::vector<std::string> t;std::stringstream ss(line);std::string tok;while(std::getline(ss,tok,d))t.push_back(trim(tok));return t;}

struct LogisticModel{std::vector<float> weights;float bias=0;int n_features=0,max_iter=100;float lr=0.01f;};

extern "C" {

void* ai_create_matrix(int r,int c){return new nexa::Tensor(std::vector<float>(r*c,0.f),{r,c});}
void  ai_set_value(void* p,int r,int c,float v){auto*t=static_cast<nexa::Tensor*>(p);t->data[r*t->shape[1]+c]=v;}
float ai_get_value(void* p,int r,int c){auto*t=static_cast<nexa::Tensor*>(p);return t->data[r*t->shape[1]+c];}
void* ai_matmul(void* a,void* b){auto*A=static_cast<nexa::Tensor*>(a);auto*B=static_cast<nexa::Tensor*>(b);nexa::Tensor r=nexa::matmul(*A,*B);return new nexa::Tensor(r.data,r.shape);}
void  ai_print(void* p){auto*t=static_cast<nexa::Tensor*>(p);int rows=t->shape[0],cols=t->shape[1];std::cout<<"[";for(int i=0;i<rows;i++){if(i>0)std::cout<<" ";std::cout<<"[";for(int j=0;j<cols;j++){std::cout<<t->data[i*cols+j];if(j<cols-1)std::cout<<", ";}std::cout<<"]";if(i<rows-1)std::cout<<",\n";}std::cout<<"]"<<std::endl;}
void* ai_zeros(int r,int c){return new nexa::Tensor(std::vector<float>(r*c,0.f),{r,c});}
void* ai_ones(int r,int c){return new nexa::Tensor(std::vector<float>(r*c,1.f),{r,c});}
float ai_sum(void* p){auto*t=static_cast<nexa::Tensor*>(p);float s=0;for(float v:t->data)s+=v;return s;}
float ai_mean(void* p){auto*t=static_cast<nexa::Tensor*>(p);if(t->data.empty())return 0;float s=0;for(float v:t->data)s+=v;return s/t->data.size();}
float ai_max(void* p){auto*t=static_cast<nexa::Tensor*>(p);if(t->data.empty())return 0;float m=t->data[0];for(float v:t->data)if(v>m)m=v;return m;}
float ai_min(void* p){auto*t=static_cast<nexa::Tensor*>(p);if(t->data.empty())return 0;float m=t->data[0];for(float v:t->data)if(v<m)m=v;return m;}
void* ai_reshape(void* p,int r,int c){auto*t=static_cast<nexa::Tensor*>(p);return new nexa::Tensor(t->data,{r,c});}
void* ai_shape(void* p){auto*t=static_cast<nexa::Tensor*>(p);return new nexa::Tensor({(float)t->shape[0],(float)t->shape[1]},{1,2});}

// CSV
void* csv_read(const char* path,int skip_header){
    std::ifstream f(path);if(!f.is_open()){fprintf(stderr,"[nexa] cannot open CSV '%s'\n",path);return nullptr;}
    std::vector<std::vector<float>> rows;std::string line;bool first=true;
    while(std::getline(f,line)){if(trim(line).empty())continue;if(first&&skip_header){first=false;continue;}first=false;
        auto toks=splitLine(line);std::vector<float> row;for(auto& tok:toks){try{row.push_back(std::stof(tok));}catch(...){row.push_back(0.f);}}if(!row.empty())rows.push_back(row);}
    if(rows.empty())return new nexa::Tensor({},{0,0});
    int nr=(int)rows.size(),nc=(int)rows[0].size();std::vector<float> data;
    for(auto& r:rows){r.resize(nc,0.f);for(float v:r)data.push_back(v);}
    return new nexa::Tensor(data,{nr,nc});
}
void  csv_write(const char* path,void* tp){auto*t=static_cast<nexa::Tensor*>(tp);if(!t)return;std::ofstream f(path);if(!f.is_open())return;int rows=t->shape[0],cols=t->shape[1];for(int i=0;i<rows;i++){for(int j=0;j<cols;j++){f<<t->data[i*cols+j];if(j<cols-1)f<<",";}f<<"\n";}}
int   csv_rows(void* p){return static_cast<nexa::Tensor*>(p)->shape[0];}
int   csv_cols(void* p){return static_cast<nexa::Tensor*>(p)->shape[1];}
float csv_get(void* p,int r,int c){auto*t=static_cast<nexa::Tensor*>(p);return t->data[r*t->shape[1]+c];}
void  csv_set(void* p,int r,int c,float v){auto*t=static_cast<nexa::Tensor*>(p);t->data[r*t->shape[1]+c]=v;}
void* csv_get_row(void* p,int row){auto*t=static_cast<nexa::Tensor*>(p);int c=t->shape[1];return new nexa::Tensor(std::vector<float>(t->data.begin()+row*c,t->data.begin()+row*c+c),{1,c});}
void* csv_get_col(void* p,int col){auto*t=static_cast<nexa::Tensor*>(p);int r=t->shape[0],c=t->shape[1];std::vector<float> d;for(int i=0;i<r;i++)d.push_back(t->data[i*c+col]);return new nexa::Tensor(d,{r,1});}
void* csv_slice_cols(void* p,int cs,int ce){auto*t=static_cast<nexa::Tensor*>(p);int r=t->shape[0],c=t->shape[1],nc=ce-cs;std::vector<float> d;for(int i=0;i<r;i++)for(int j=cs;j<ce&&j<c;j++)d.push_back(t->data[i*c+j]);return new nexa::Tensor(d,{r,nc});}

// ML ops
void* ml_normalize(void* p){
    auto*t=static_cast<nexa::Tensor*>(p);int rows=t->shape[0],cols=t->shape[1];std::vector<float> out=t->data;
    for(int j=0;j<cols;j++){float mn=out[j],mx=out[j];for(int i=1;i<rows;i++){float v=out[i*cols+j];if(v<mn)mn=v;if(v>mx)mx=v;}float rng=mx-mn;if(rng==0)rng=1;for(int i=0;i<rows;i++)out[i*cols+j]=(out[i*cols+j]-mn)/rng;}
    return new nexa::Tensor(out,{rows,cols});
}
void* ml_shuffle(void* p){
    auto*t=static_cast<nexa::Tensor*>(p);int rows=t->shape[0],cols=t->shape[1];std::vector<float> out=t->data;
    srand((unsigned)time(nullptr));for(int i=rows-1;i>0;i--){int j=rand()%(i+1);for(int c=0;c<cols;c++)std::swap(out[i*cols+c],out[j*cols+c]);}
    return new nexa::Tensor(out,{rows,cols});
}
void* ml_train_split(void* p,float ratio){auto*t=static_cast<nexa::Tensor*>(p);int rows=t->shape[0],cols=t->shape[1],n=(int)(rows*ratio);std::vector<float> d(t->data.begin(),t->data.begin()+n*cols);return new nexa::Tensor(d,{n,cols});}
void* ml_test_split(void* p,float ratio){auto*t=static_cast<nexa::Tensor*>(p);int rows=t->shape[0],cols=t->shape[1],n=(int)(rows*ratio);std::vector<float> d(t->data.begin()+n*cols,t->data.end());return new nexa::Tensor(d,{rows-n,cols});}
void* ml_hstack(void* ap,void* bp){auto*A=static_cast<nexa::Tensor*>(ap);auto*B=static_cast<nexa::Tensor*>(bp);int rows=A->shape[0],ca=A->shape[1],cb=B->shape[1];std::vector<float> out;for(int i=0;i<rows;i++){for(int j=0;j<ca;j++)out.push_back(A->data[i*ca+j]);for(int j=0;j<cb;j++)out.push_back(B->data[i*cb+j]);}return new nexa::Tensor(out,{rows,ca+cb});}

// Logistic Regression
void* lore_create(int max_iter,float lr){auto*m=new LogisticModel();m->max_iter=max_iter;m->lr=lr;return m;}
void  lore_fit(void* mp,void* Xp,void* yp){
    auto*model=static_cast<LogisticModel*>(mp);auto*X=static_cast<nexa::Tensor*>(Xp);auto*y=static_cast<nexa::Tensor*>(yp);
    int n=X->shape[0],nf=X->shape[1];model->n_features=nf;model->weights.assign(nf,0.f);model->bias=0.f;
    for(int iter=0;iter<model->max_iter;iter++){std::vector<float> dw(nf,0.f);float db=0.f;
        for(int i=0;i<n;i++){float z=model->bias;for(int j=0;j<nf;j++)z+=model->weights[j]*X->data[i*nf+j];
            float err=sigmoid(z)-y->data[i];for(int j=0;j<nf;j++)dw[j]+=err*X->data[i*nf+j];db+=err;}
        for(int j=0;j<nf;j++)model->weights[j]-=model->lr*dw[j]/n;model->bias-=model->lr*db/n;}
}
void* lore_predict(void* mp,void* Xp){
    auto*model=static_cast<LogisticModel*>(mp);auto*X=static_cast<nexa::Tensor*>(Xp);
    int n=X->shape[0],nf=X->shape[1];std::vector<float> out(n);
    for(int i=0;i<n;i++){float z=model->bias;for(int j=0;j<nf;j++)z+=model->weights[j]*X->data[i*nf+j];out[i]=sigmoid(z)>=0.5f?1.f:0.f;}
    return new nexa::Tensor(out,{n,1});
}
void* lore_predict_proba(void* mp,void* Xp){
    auto*model=static_cast<LogisticModel*>(mp);auto*X=static_cast<nexa::Tensor*>(Xp);
    int n=X->shape[0],nf=X->shape[1];std::vector<float> out(n);
    for(int i=0;i<n;i++){float z=model->bias;for(int j=0;j<nf;j++)z+=model->weights[j]*X->data[i*nf+j];out[i]=sigmoid(z);}
    return new nexa::Tensor(out,{n,1});
}
float ml_accuracy(void* predp,void* labelp){
    auto*pred=static_cast<nexa::Tensor*>(predp);auto*label=static_cast<nexa::Tensor*>(labelp);
    int n=(int)pred->data.size();if(n==0)return 0.f;int correct=0;
    for(int i=0;i<n;i++)if(std::round(pred->data[i])==std::round(label->data[i]))correct++;
    return(float)correct/n;
}
void* ml_confusion(void* predp,void* labelp){
    auto*pred=static_cast<nexa::Tensor*>(predp);auto*label=static_cast<nexa::Tensor*>(labelp);
    int n=(int)pred->data.size();float tp=0,fp=0,fn=0,tn=0;
    for(int i=0;i<n;i++){int p=(int)std::round(pred->data[i]),l=(int)std::round(label->data[i]);if(p==1&&l==1)tp++;else if(p==1&&l==0)fp++;else if(p==0&&l==1)fn++;else tn++;}
    return new nexa::Tensor({tp,fp,fn,tn},{2,2});
}
void* lore_weights(void* mp){auto*m=static_cast<LogisticModel*>(mp);return new nexa::Tensor(m->weights,{1,m->n_features});}
float lore_bias(void* mp){return static_cast<LogisticModel*>(mp)->bias;}

} // extern "C"