#include "password_strength.h"
static bool sequence(const String&s){ for(size_t i=0;i+2<s.length();i++){ char a=s[i],b=s[i+1],c=s[i+2]; if((b==a+1&&c==b+1)||(b==a-1&&c==b-1)) return true; } return false; }
PasswordReport analyzePassword(const String& p){
  bool lower=false,upper=false,digit=false,symbol=false; for(char c:p){ if(c>='a'&&c<='z')lower=true; else if(c>='A'&&c<='Z')upper=true; else if(c>='0'&&c<='9')digit=true; else symbol=true; }
  int pool=(lower?26:0)+(upper?26:0)+(digit?10:0)+(symbol?33:0); float e=pool? p.length()*log2((double)pool):0;
  bool common=(p.length()<6 || p.equalsIgnoreCase("password") || p.equalsIgnoreCase("123456") || p.equalsIgnoreCase("qwerty") || p.equalsIgnoreCase("admin"));
  bool seq=sequence(p); uint8_t score=0; if(p.length()>=8)score+=25; if(p.length()>=12)score+=20; if(lower&&upper)score+=15; if(digit)score+=15; if(symbol)score+=15; if(common)score=score>35?score-35:0; if(seq)score=score>15?score-15:0;
  return {score,e,common,seq};
}
String passwordAdvice(const PasswordReport&r){ if(r.score<40)return "Use uma senha maior, única e com mistura de classes de caracteres."; if(r.score<70)return "Aumente o comprimento e evite sequências e padrões previsíveis."; return "Boa composição. Prefira ainda uma senha longa e exclusiva para cada serviço."; }
