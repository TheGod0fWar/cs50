// Trie - CS50 Week 5
// Stores names and phone numbers using a trie (retrieval tree).
// This is the same concept David Malan teaches:
//   - Each node has 26 children (one per letter A-Z)
//   - We walk down the tree letter by letter
//   - At the end of a name, we store the phone number
//   - Search is O(k) where k = length of the name (NOT the number of names!)

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET_SIZE 26
#define PHONE_LEN 20

// Each node in the trie has:
//   - An array of 26 children (one for each letter)
//   - A phone number (only filled in at the END of a name)
typedef struct node {
  char phone[PHONE_LEN]; // "" if this node is NOT the end of a name
  struct node *children[ALPHABET_SIZE];
} node;

// The root of our trie
node *root = NULL;

// Create a new empty trie node
node *create_node(void) {
  node *n = malloc(sizeof(node));
  if (n == NULL) {
    return NULL;
  }
  n->phone[0] = '\0'; // No phone number stored here (yet)
  for (int i = 0; i < ALPHABET_SIZE; i++) {
    n->children[i] = NULL;
  }
  return n;
}

// Insert a name and phone number into the trie
bool insert(const char *name, const char *phone) {
  printf("  Inserting \"%s\" -> \"%s\":\n", name, phone);

  // Start at the root
  node *cursor = root;

  // Walk through each letter of the name
  for (int i = 0, len = strlen(name); i < len; i++) {
    // Convert letter to index 0-25
    int index = toupper(name[i]) - 'A';

    // If the child doesn't exist yet, create it
    if (cursor->children[index] == NULL) {
      cursor->children[index] = create_node();
      if (cursor->children[index] == NULL) {
        return false;
      }
      printf("    [NEW NODE] created for '%c' (index %i)\n", toupper(name[i]),
             index);
    } else {
      printf("    [EXISTING] node for '%c' (index %i) already exists\n",
             toupper(name[i]), index);
    }

    // Move to the child
    cursor = cursor->children[index];
  }

  // We've reached the end of the name - store the phone number here
  strcpy(cursor->phone, phone);
  printf("    [STORED] phone \"%s\" at end of \"%s\"\n\n", phone, name);
  return true;
}

// Search for a name and return the phone number
const char *search(const char *name) {
  printf("  Searching for \"%s\":\n", name);

  node *cursor = root;
  int steps = 0;

  // Walk through each letter of the name
  for (int i = 0, len = strlen(name); i < len; i++) {
    int index = toupper(name[i]) - 'A';
    steps++;

    printf("    Step %i: looking for '%c' (index %i)... ", steps,
           toupper(name[i]), index);

    // If there's no child for this letter, name doesn't exist
    if (cursor->children[index] == NULL) {
      printf("NOT FOUND! Dead end.\n");
      return NULL;
    }

    printf("found!\n");
    cursor = cursor->children[index];
  }

  // We've walked through all letters - check if a phone is stored here
  if (cursor->phone[0] != '\0') {
    printf("    [FOUND] after %i steps (length of the name, NOT total names "
           "stored!)\n",
           steps);
    return cursor->phone;
  }

  printf("    [NOT FOUND] reached end but no phone stored (prefix of another "
         "name)\n");
  return NULL;
}

// Recursively print the trie structure (for visualization)
void print_trie_helper(node *n, char *prefix, int depth) {
  if (n == NULL) {
    return;
  }

  // If this node has a phone number, print it
  if (n->phone[0] != '\0') {
    printf("  %s -> %s\n", prefix, n->phone);
  }

  // Recurse into each child
  for (int i = 0; i < ALPHABET_SIZE; i++) {
    if (n->children[i] != NULL) {
      prefix[depth] = 'A' + i;
      prefix[depth + 1] = '\0';
      print_trie_helper(n->children[i], prefix, depth + 1);
    }
  }
}

void print_trie(void) {
  printf("\n========== TRIE CONTENTS ==========\n");
  char prefix[100] = "";
  print_trie_helper(root, prefix, 0);
  printf("===================================\n\n");
}

// Recursively free all nodes in the trie
void free_trie(node *n) {
  if (n == NULL) {
    return;
  }
  for (int i = 0; i < ALPHABET_SIZE; i++) {
    free_trie(n->children[i]);
  }
  free(n);
}

int main(void) {
  // Create the root node
  root = create_node();
  if (root == NULL) {
    printf("Could not create trie\n");
    return 1;
  }

  printf("=== TRIE DEMO (CS50 Week 5) ===\n\n");
  printf("How it works:\n");
  printf("  1. Each node has 26 children (one per letter A-Z)\n");
  printf("  2. We walk DOWN the tree, one letter at a time\n");
  printf("  3. At the end of a name, we store the phone number\n");
  printf("  4. Search time = O(length of name), NOT O(number of names)!\n");
  printf("     This is what makes tries special.\n\n");

  // ----- INSERTING NAMES -----
  printf("--- INSERTING NAMES ---\n\n");

  insert("Harry", "617-495-1000");
  insert("Hermione", "617-495-2000");
  printf("  ^ Notice: Harry and Hermione SHARE the 'H' node!\n");
  printf("    No collision - they branch at the 2nd letter (A vs E).\n");
  printf("    This is the advantage over a hash table!\n\n");

  insert("Hagrid", "617-495-7000");
  printf("  ^ Hagrid shares H->A with Harry, then branches at 'G' vs 'R'.\n\n");

  insert("Ron", "617-495-3000");
  insert("Draco", "617-495-4000");
  insert("Luna", "617-495-5000");
  insert("Dobby", "617-495-6000");

  // Show the trie contents
  print_trie();

  // ----- SEARCHING -----
  printf("--- SEARCHING FOR NAMES ---\n\n");

  // Search for a name that shares a prefix with others
  printf("Searching for \"Harry\":\n");
  const char *phone = search("Harry");
  if (phone != NULL) {
    printf("  => Harry's phone: %s\n\n", phone);
  }

  printf("Searching for \"Hermione\":\n");
  phone = search("Hermione");
  if (phone != NULL) {
    printf("  => Hermione's phone: %s\n\n", phone);
  }

  printf("Searching for \"Ron\":\n");
  phone = search("Ron");
  if (phone != NULL) {
    printf("  => Ron's phone: %s\n\n", phone);
  }

  // Search for someone who doesn't exist
  printf("Searching for \"Voldemort\":\n");
  phone = search("Voldemort");
  if (phone == NULL) {
    printf("  => Voldemort not found in the trie!\n\n");
  }

  // Search for a prefix that exists but isn't a full name
  printf(
      "Searching for \"Ha\" (prefix of Harry/Hagrid but not a stored name):\n");
  phone = search("Ha");
  if (phone == NULL) {
    printf("  => \"Ha\" is not a complete name in the trie!\n\n");
  }

  // ----- COMPARISON -----
  printf("--- TRIE vs HASH TABLE ---\n");
  printf("  Hash Table:\n");
  printf("    - Array of linked lists\n");
  printf("    - Ideal: O(1) search, but collisions make it O(n)\n");
  printf("    - Harry & Hermione COLLIDE (same first letter)\n\n");
  printf("  Trie:\n");
  printf("    - Tree where each node has 26 children\n");
  printf("    - Search is ALWAYS O(k) where k = length of name\n");
  printf("    - Harry & Hermione DON'T collide - they share a prefix\n");
  printf("    - Downside: uses more memory (each node has 26 pointers)\n\n");

  // Free all memory
  free_trie(root);

  return 0;
}
