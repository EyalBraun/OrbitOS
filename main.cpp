#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;
//
string commands[64];

void compile_command(string c){
  // the index is = ((f*31) + l ) %64
  // the f = first letter , l = last letter.
  
  
}
void base_display(string name,string dir){
  cout<<name<<":"<<dir<<endl;
}
int get_idx(string s){
  return ((s[0]*31) + s[s.size()-1]) %64;
  
}
void build_commands(){
  fstream file("commands.txt");
string line;
while (getline(file, line)) {
       int idx = get_idx(line);
        commands[idx] = line;

     
    }

}
int main(){
  build_commands();
  
  string name; cout<<"Whats the Owner Name ? : "; cin >> name;
  string dir = "root";
  for(int i = 0;i<64;i++){
    if(commands[i] != "")cout<<commands[i] << endl;
  }
  while(true){
        base_display(name,dir);

        string com; cin >> com; 
      if(com == "kill") break;


        
    }
  return 0;
    
  }
  


   



  
