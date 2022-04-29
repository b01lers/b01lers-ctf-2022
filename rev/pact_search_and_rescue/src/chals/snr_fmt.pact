class Node {
  init(value) {
    this.left = nil;
    this.right = nil;
    this.value = value;
    this.freq = 0;
  }
}

fun insert_in_order(head, val) {
  var newNode = Node(val);
  newNode.freq = val.freq;
  if (head == nil) {
    return newNode;
  }

  var cur = head;
  while (cur.right != nil && cur.right.freq < newNode.freq) {
    cur = cur.right;
  }
  newNode.right = cur.right;
  cur.right = newNode;
  return head;
}

fun build_tree(freqs) {
  var list_head = nil;
  for (var i = 0; i < len(freqs); i = i + 1) {
    if (freqs[i] != 0) {
      var new = Node(chr(i));
      new.freq = freqs[i];
      list_head = insert_in_order(list_head, new);
    }
  }
  while (list_head != nil && list_head.right != nil) {
    var left = list_head.value;
    list_head = list_head.right;
    var right = list_head.value;
    list_head = list_head.right;
    var internal = Node(0);
    internal.freq = left.freq + right.freq;
    internal.left = left;
    internal.right = right;
    list_head = insert_in_order(list_head, internal);
  }
  return list_head.value;
}

fun create_freqs() {
  var freqs = alloc(256);

  var num_freqs = [16, 18, 17, 21, 15, 10, 20, 15, 16, 11];
  var lwr_freqs = [12, 12, 18, 17, 13, 13, 18, 9, 12, 24, 17, 23, 12, 19, 12, 17, 10, 9, 17, 17, 7, 18, 26, 14, 16, 14];
  var upr_freqs = [13, 9, 8, 15, 23, 14, 22, 20, 17, 10, 12, 14, 14, 20, 11, 13, 20, 16, 12, 14, 16, 22, 14, 13, 20, 22];

  for(var i = ord("0"); i <= ord("9"); i = i + 1) {
    freqs[i] = num_freqs[i - ord("0")];
  }

  for(var i = ord("a"); i <= ord("z"); i = i + 1) {
    freqs[i] = lwr_freqs[i - ord("a")];
  }

  for(var i = ord("A"); i <= ord("Z"); i = i + 1) {
    freqs[i] = upr_freqs[i - ord("A")];
  }
  freqs[ord("{")] = 18;
  freqs[ord("}")] = 12;
  freqs[ord("_")] = 11;
  return freqs;
}
// Get around the constant limit
var FREQS = create_freqs();
var KEY = "yt{d68pmRmTb1do1Ijw2h";
var path = input("> ");
var root = build_tree(FREQS);
var cur = root;
var key_check = [];

for (var i = 0; i < len(path); i = i + 1) {
  var cur_char = ord(path[i]);
  for (var j = 0; j < 8; j = j + 1) {
    if (cur.left == nil && cur.right == nil) {
      append(key_check, cur.value);
      cur = root;
    }
    if ((cur_char & 1) == 1) {
      cur = cur.right;
    } else {
      cur = cur.left;
    }
    cur_char = cur_char >> 1;
  }
}

if (len(key_check) != len(KEY)) {
  exit(1);
}
for(var i = 0; i < len(KEY); i = i + 1) {
  if(key_check[i] != KEY[i]) {
  print key_check[i];
  print KEY[i];
    exit(1);
  }
}
exit(0);
