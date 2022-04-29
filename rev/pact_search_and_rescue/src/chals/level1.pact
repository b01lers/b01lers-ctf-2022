//bctf{are_you_satisfied_with_this_vm}
fun main() {
    var user_input = split(input("Flag Validator: "));
    if (len(user_input) != 36) { return 1; }
    var num_input = range(len(user_input));
    for (var i = 0; i < len(user_input); i = i + 1) {
        num_input[i] = int(user_input[i]);
    }
  if ((num_input[1] - num_input[15]) != -17) { return 1; }
  if ((num_input[25] ^ num_input[22]) != 43) { return 1; }
  if ((num_input[19] - num_input[14]) != 8) { return 1; }
  if ((num_input[29] - num_input[34]) != -5) { return 1; }
  if ((num_input[23] + num_input[21]) != 219) { return 1; }
  if ((num_input[24] + num_input[12]) != 200) { return 1; }
  if ((num_input[35] ^ num_input[25]) != 9) { return 1; }
  if ((num_input[14] ^ num_input[27]) != 62) { return 1; }
  if ((num_input[22] + num_input[8]) != 190) { return 1; }
  if ((num_input[3] + num_input[26]) != 206) { return 1; }
  if ((num_input[32] ^ num_input[34]) != 50) { return 1; }
  if ((num_input[21] ^ num_input[23]) != 19) { return 1; }
  if ((num_input[7] + num_input[10]) != 212) { return 1; }
  if ((num_input[2] + num_input[10]) != 227) { return 1; }
  if ((num_input[17] - num_input[35]) != -10) { return 1; }
  if ((num_input[5] + num_input[18]) != 199) { return 1; }
  if ((num_input[15] ^ num_input[1]) != 23) { return 1; }
  if ((num_input[30] ^ num_input[31]) != 26) { return 1; }
  if ((num_input[18] - num_input[10]) != -9) { return 1; }
  if ((num_input[9] - num_input[19]) != 16) { return 1; }
  if ((num_input[31] + num_input[8]) != 210) { return 1; }
  if ((num_input[4] - num_input[26]) != 19) { return 1; }
  if ((num_input[10] - num_input[9]) != -10) { return 1; }
  if ((num_input[13] + num_input[5]) != 212) { return 1; }
  if ((num_input[6] ^ num_input[13]) != 1) { return 1; }
  if ((num_input[28] ^ num_input[20]) != 17) { return 1; }
  if ((num_input[34] - num_input[30]) != 4) { return 1; }
  if ((num_input[11] ^ num_input[2]) != 1) { return 1; }
  if ((num_input[16] + num_input[11]) != 222) { return 1; }
  if ((num_input[8] ^ num_input[18]) != 57) { return 1; }
  if ((num_input[20] ^ num_input[0]) != 7) { return 1; }
  if ((num_input[27] ^ num_input[28]) != 43) { return 1; }
  if ((num_input[26] - num_input[17]) != -11) { return 1; }
  if ((num_input[12] ^ num_input[31]) != 44) { return 1; }
  if ((num_input[33] - num_input[8]) != 23) { return 1; }
  if ((num_input[0] + num_input[21]) != 198) { return 1; }
  return 0;
}
if (main() == 0) {
    print "Congrats!";
    exit(0);
} else {
    print "Try again :(";
    exit(1);
}
