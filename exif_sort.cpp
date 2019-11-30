#include <windows.h>
#include <dirent.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
using namespace std;


struct picture{
  string file_name;
  string picture_num;
};
struct day{
  string day_num;                                    //day of pic submitted
  vector<picture> pic_number;                      //vector of all pics submitted on day_num
};
struct subject{
  string subj_id;                                  //subject id
  vector<day> subj_day;                     //vector of all days subject submitted picture
};
int fail_counter=0;
vector<subject> checked;

string parse_path(string in_path){
  for(int i = 0; i<in_path.length();i++){
    if(in_path[i] == '\\'){
      in_path.insert(i, "\\");
      i++;
    }
  }
  return in_path;
}
bool sort_func(day day1, day day2){
  return day1.day_num<day2.day_num;
}
void id_separator(string filename){
  if(filename != "." && filename != ".."){
    stringstream name(filename);
    string id, in_day, unnecessary, pic_num, type;
    getline(name,id,'_');
    getline(name, unnecessary, '_');
    getline(name, in_day, '_');
    getline(name, unnecessary, '_');
    getline(name, unnecessary, '_');
    getline(name, pic_num, '_');
    getline(name, unnecessary,'.');
    getline(name, type);
    pic_num = (pic_num.substr(0,4));

    picture temp_pic;                     //picture info , with the picture number for that day amd picture file name
    temp_pic.picture_num = pic_num;
    temp_pic.file_name = filename;

    day temp_day;                         //temporary day with the day number and a pic
    temp_day.day_num = in_day;
    temp_day.pic_number.push_back(temp_pic);

    subject temp;
    temp.subj_id = id;

    int checked_len = checked.size();
    bool found = false;
    for(int i = 0; i<checked_len; i++){ //checks if subject has already had a photo analyzed
      if(temp.subj_id == checked[i].subj_id){//if subject has already had a photo added
        bool day_found = false;
        int subj_day_length = checked[i].subj_day.size();
        for(int j = 0; j<subj_day_length; j++){//check if photo's day has been added before
          if(checked[i].subj_day[j].day_num == in_day){//if it has, add new picture for that day
            checked[i].subj_day[j].pic_number.push_back(temp_pic);
            day_found = true;
          }
        }
        if(day_found == false){//otherwise, add pic as first pic of a new day
          checked[i].subj_day.push_back(temp_day);
        }
        found = true;
      }
    }
    if(found == false){//if this photo is subject's first photo, adds pic_num
      temp.subj_day.push_back(temp_day);
      checked.push_back(temp);
    }
  }
}
void sort_by_day(){
  int checked_length = checked.size();
  for(int i = 0; i<checked_length; i++){
    std::sort(checked[i].subj_day.begin(), checked[i].subj_day.end(), sort_func);
  }
}
void create_directory(subject in_subject, string in_path, string dest_path){
  string subj_path = dest_path + "\\Subject_";
  subj_path.append(in_subject.subj_id);
  CreateDirectory(subj_path.c_str(),NULL);//creates subject folder

  string addendum = "\\Day_";
  string og_path = subj_path+addendum;
  for(int i = 0; i<in_subject.subj_day.size();i++){
    string copy = og_path;
    copy.append(in_subject.subj_day[i].day_num);
    CreateDirectory(copy.c_str(), NULL);
    copy = copy+"\\";
    for(int j = 0; j<in_subject.subj_day[i].pic_number.size(); j++){
      string changed = copy;
      changed = changed+in_subject.subj_day[i].pic_number[j].picture_num+".jpeg";
      string in_path_copy = in_path;
      in_path_copy = in_path_copy+"\\"+in_subject.subj_day[i].pic_number[j].file_name;
      int result = CopyFile(in_path_copy.c_str(), changed.c_str(), false);
      if(result == 0){
        cout<<"\tFAILED: ";
        cout<<GetLastError()<<endl;
        fail_counter++;
      }
    }
  }
}
bool checked_sort(subject s1, subject s2){
  return stoi(s1.subj_id)<stoi(s2.subj_id);
}
void sort_by_subj(){
  std::sort(checked.begin(), checked.end(), checked_sort);
}

int main(){
  cout<<"Enter a file path to sort pictures into: "<<endl;
  string to_path;
  getline(cin, to_path);
  string parsed_to = parse_path(to_path);
  CreateDirectory(parsed_to.c_str(),NULL);

  cout<<"Enter file path to directory containing unsorted photos"<<endl;
  string from_path;
  getline(cin, from_path);
  string parsed_from = parse_path(from_path);

  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir (parsed_from.c_str())) != NULL) {
      while ((ent = readdir (dir)) != NULL) {
        id_separator(ent->d_name);
      }
      closedir (dir);
  }
  else {
    /* could not open directory */
    perror ("File failed to open");
    return EXIT_FAILURE;
  }

  sort_by_day();
  sort_by_subj();

  for(int i = 0; i<checked.size(); i++){
    cout<<"Sorting photos for subject "<<checked[i].subj_id<<"..."<<endl;
    create_directory(checked[i], parsed_from, parsed_to);
  }
  if(fail_counter !=0){
    cout<<fail_counter<<" files failed to copy."<<endl;
  }
  else{
    cout<<endl<<"All files successfully sorted!"<<endl;
  }
  return 0;
}
