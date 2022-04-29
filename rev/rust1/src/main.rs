use std::str;
use std::process::Command;
use rand::{thread_rng, Rng};
#[derive(Debug)]
struct Monster {
    battle_message: String,
    health: i32
}

impl Monster{
    fn intro(&self) {
        println!("{}",self.battle_message);
        println!("I have {} health, prepare to lose",self.health);
    }

}
fn monster3() -> bool{
    //keygenme, with z3
    println!("Prepare to lose");
    let art = r#"
    __,-----,,,,  ,,,--------,__ 
    _-/|\\/|\\/|\\\|\//\\\//|/|//|\\_ 
   /|\/\//\\\\\\\\\\//////////////\\\\ 
 //|//           \\\///            |\\|\ 
///|\/             \/               \|\|\ 
|/|//                                 |\\|\  
|/|/                                    \|\|\
///;    ,,=====,,,  ~~-~~  ,,,=====,,    ;|\|\
|/|/   '"          `'     '"          "'   ;|\|
||/`;   _--~~~~--__         __--~~~~--_   ;/|\|
/|||;  :  /       \~~-___-~~/       \  :  ;|\| 
/\|;    -_\  (o)  / ,'; ;', \  (o)  /_-    ;|| 
|\|;      ~-____--~'  ; ;  '~--____-~      ;\| 
||;            ,`   ;   ;   ',            ;||
__|\ ;        ,'`    (  _  )    `',        ;/|__ 
_,-~   \|/;    ,'`        ~~ ~~        `',    ;|\   ~-,_ 
,'         ||;  '                           '  ;\|/       `, 
, _          ; ,         _--~~-~~--_           ;            _',
,-' `;-,        ;        ,; |_| | |_| ;,       ;;        ,-;' `-,
; `,      ;       ;_| : `~'~' : |_;       ;      ,' ;
;  `,     ;     :  `\/       \/   :     ;     ,'  ;
;   `,    ;     :               ;     ;    ,'   ;
;    `,_  ;     ;./\_     _/\.;     ;   _,    ;
spb   _-'        ;  ;     ~~--|~|~|--~~     ;   ;       '-_
_,-'            ;  ;        ~~~~~        ;   ;           `-,_
,~                 ;  \`~--__         __--~/  ;                ~,
        ;   \   ~~-----~~    /   ;                   
         ~-_  \  /  |  \   /  _-~                    
            ~~-\/   |   \/ -~~                       
           (=)=;==========;=(=)
    "#;
    println!("{}", art);
    let mut counter = 10;
    let mut player_health = 70;
    let mut rng = thread_rng();
    let mut m1 = Monster{
        battle_message: "Fee-fi-fo-fum\n".to_string(),
        health:120
    };
    m1.intro();
    while counter > 1 {
        if m1.health == 0{
            println!("I CAN'T BELIEVE IT!!!!\n You beat me");
            println!("BUT WHERE'S THE FLAG???");
            return false;
        }
        else{
            if player_health == 0{ 
                println!("HAHAHAHA YOU LOSE!!!!");
                return false;
            }
        }
        // 1 -> Whatever RNG draws
        // 2 -> RNG DIV 2
        // 3 -> RNG DIV 3
        let mut mod_option = 1;
        println!("1) Flame");
        println!("2) Ice");
        println!("3) Wind");
        println!("Enter your attack");
        let mut stuff = String::new();
        std::io::stdin().read_line(&mut stuff).unwrap();
        let attack:i32 = stuff.trim().parse().expect("Bruh....you have to enter an attack.");
        match attack {
            1 => mod_option = 1,
            2 => mod_option = 2,
            3 => mod_option = 3,
            _ => panic!("Illegal attack")
        }
        println!("Any powerups?");
        println!("Y/N");
        let mut choice = String::new();
        std::io::stdin().read_line(&mut choice).unwrap();
        let choice = choice.trim();
        if choice == "Y".to_string(){
            println!("Enter your powerup");
            let mut stuff = String::new();
            std::io::stdin().read_line(&mut stuff).unwrap();
            if stuff.trim().len() != 22{
                panic!("Illegal length");
            }
            let number = stuff.as_bytes();
            let mut check1:i64 = 0; 
            for i in 0..4{
                check1 += number[i] as i64;
            }
            if check1 != 0x115{
                println!("Check1 failed");
                return false;
            }
            let check2:i64 = number[4] as i64 * number[5] as i64
            + number[6] as i64 + number[7] as i64;
            if check2 != 0x1337{
                return false;
            }
            for i in 0..number.len()-1{
                if number[i] < 0x21 || number[i] > 0x7e{
                    return false;
                }
            }
            let check4 =( number[8] + number[9]) as i64 >= 100 && number[10] / number[11] == 2 
            && (number[12] as i64 * number[13] as i64) > 1000 && (number[14] as i64 +number[15] as i64)*5 > 200 && 
            number[16] - number[17] == 42 && number[18] - number[19] == 2 && 
            (number[20] as i64 * number[21] as i64) >= 900;
            if check4 {
                print!("Ok....");
                return true; 
            }
        }
        else if choice != "N".to_string(){
            panic!("Choose more carefully");
        }
        let monster_roll:i32 = rng.gen_range(10..30);
        let player_roll = rng.gen_range(10..30) / mod_option;
        println!("You rolled: {}", player_roll);
        println!("Monster rolled: {}", monster_roll);
        if monster_roll > player_roll{
            player_health = player_health - monster_roll;
            if player_health < 0 {
                player_health = 0; //cleanup
            }
            println!("Nice try ;)");
        }
        else if player_roll > monster_roll {
            m1.health = m1.health - player_roll;
            if m1.health < 0 {
                m1.health = 0; // cleanup 
            }
            println!("Ouch! That's gonna leave a mark");
        }
        else{
            println!("Ooooh a tie, let's do it again");
        }
        println!("You now have {} health\n and the monster has {} health", player_health, m1.health);
        counter = counter - 1;
    }
    println!("Ran out of attacks, try again later");
    return false;
}
fn main() {
    println!("To get the flag, you have to beat the monster the correct way");
    println!("Think you can handle it?....");
    let m = monster3();
    if m {
        let output = Command::new("cat")
        .arg("flag.txt")
        .output()
        .expect("Uh oh, flag file is missing, contact an admin");
        println!("{:?}", str::from_utf8(output.stdout.as_slice()));
    }
}
