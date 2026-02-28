#include <iostream>
#include <string>
#include <vector>
using namespace std;
//all of the commands functions in an array.
string commands_functions[64];
// show the base UserName and curr Dir.

void base_display(string name, string dir){
  cout<<name<<":"<<dir<<endl;
}

void compile_command(string c){
  // the index is = ((f*31) + l ) %64
  // the f = first letter , l = last letter.
  
  
}
int main(){
  
  
  string name; cout<<"Whats the Owner Name ? : "; cin >> name;
  string dir = "root";
  while(true){
        base_display(name,dir);

        string com; cin >> com; 
      if(com == "kill") break;


        
    }
  return 0;
    
  }
  



  
