// Hash Table - CS50 Week 5
// Stores names and phone numbers using a hash table.
// This is the same concept David Malan teaches:
//   - We hash a name to get an index into an array
//   - Each array slot holds a linked list (to handle collisions)
//   - We can insert and search in (ideally) O(1) time

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Size of the hash table (26 buckets, one per letter A-Z)
#define TABLE_SIZE 26
#define NAME_LEN 50
#define PHONE_LEN 20

// A node in our linked list (for chaining collisions)
typedef struct node {
  char name[NAME_LEN];
  char phone[PHONE_LEN];
  struct node *next;
} node;

// The hash table: an array of pointers to linked lists
node *table[TABLE_SIZE];

// Hash function: returns 0-25 based on the first letter of the name
// This is exactly the simple hash function from lecture
unsigned int hash(const char *name) { return toupper(name[0]) - 'A'; }

// Insert a name and phone number into the hash table
bool insert(const char *name, const char *phone) {
  // Step 1: Hash the name to get the index
  unsigned int index = hash(name);
  printf("  [HASH] \"%s\" -> bucket %i\n", name, index);

  // Step 2: Create a new node
  node *n = malloc(sizeof(node));
  if (n == NULL) {
    return false;
  }
  strcpy(n->name, name);
  strcpy(n->phone, phone);

  // Step 3: Insert at the HEAD of the linked list (O(1))
  // This is what David Malan shows: prepend to the list
  n->next = table[index];
  table[index] = n;

  printf("  [INSERT] Added \"%s\" -> \"%s\" at bucket %i\n", name, phone,
         index);
  return true;
}

// Search for a name and return the phone number
const char *search(const char *name) {
  // Step 1: Hash the name to find the right bucket
  unsigned int index = hash(name);
  printf("  [HASH] \"%s\" -> bucket %i\n", name, index);

  // Step 2: Walk through the linked list at that bucket
  int steps = 0;
  for (node *cursor = table[index]; cursor != NULL; cursor = cursor->next) {
    steps++;
    if (strcasecmp(cursor->name, name) == 0) {
      printf("  [FOUND] after %i step(s) in the linked list\n", steps);
      return cursor->phone;
    }
  }

  printf("  [NOT FOUND] searched %i node(s)\n", steps);
  return NULL;
}

// Print the entire hash table (to visualize the structure)
void print_table(void) {
  printf("\n========== HASH TABLE ==========\n");
  for (int i = 0; i < TABLE_SIZE; i++) {
    printf("[%2i] %c: ", i, 'A' + i);
    if (table[i] == NULL) {
      printf("---");
    } else {
      for (node *cursor = table[i]; cursor != NULL; cursor = cursor->next) {
        printf("{%s: %s}", cursor->name, cursor->phone);
        if (cursor->next != NULL) {
          printf(" -> ");
        }
      }
    }
    printf("\n");
  }
  printf("================================\n\n");
}

// Free all memory in the hash table
void free_table(void) {
  for (int i = 0; i < TABLE_SIZE; i++) {
    node *cursor = table[i];
    while (cursor != NULL) {
      node *tmp = cursor;
      cursor = cursor->next;
      free(tmp);
    }
    table[i] = NULL;
  }
}

int main(void) {
  // Initialize all buckets to NULL
  for (int i = 0; i < TABLE_SIZE; i++) {
    table[i] = NULL;
  }

  printf("=== HASH TABLE DEMO (CS50 Week 5) ===\n\n");
  printf("How it works:\n");
  printf("  1. We hash a name (first letter -> index 0-25)\n");
  printf("  2. We store the name+phone in a linked list at that index\n");
  printf("  3. Collisions: multiple names in the same bucket form a chain\n\n");

  // ----- INSERTING NAMES -----
  printf("--- INSERTING NAMES ---\n\n");

  insert("Harry", "617-495-1000");
  printf("\n");
  insert("Hermione", "617-495-2000");
  printf("  ^ Notice: Harry and Hermione COLLIDE (both hash to bucket %i)!\n",
         hash("Harry"));
  printf("    Hermione is prepended to the same linked list.\n\n");
  insert("Ron", "617-495-3000");
  printf("\n");
  insert("Draco", "617-495-4000");
  printf("\n");
  insert("Luna", "617-495-5000");
  printf("\n");
  insert("Dobby", "617-495-6000");
  printf("  ^ Draco and Dobby also COLLIDE (both start with 'D')!\n\n");

  // Show the full table
  print_table();

  // ----- SEARCHING -----
  printf("--- SEARCHING FOR NAMES ---\n\n");

  // Search for someone in a bucket with collisions
  printf("Searching for \"Harry\":\n");
  const char *phone = search("Harry");
  if (phone != NULL) {
    printf("  => Harry's phone: %s\n\n", phone);
  }

  // Search for someone who is first in the chain
  printf("Searching for \"Hermione\":\n");
  phone = search("Hermione");
  if (phone != NULL) {
    printf("  => Hermione's phone: %s\n\n", phone);
  }

  // Search for someone alone in their bucket
  printf("Searching for \"Ron\":\n");
  phone = search("Ron");
  if (phone != NULL) {
    printf("  => Ron's phone: %s\n\n", phone);
  }

  // Search for someone who doesn't exist
  printf("Searching for \"Voldemort\":\n");
  phone = search("Voldemort");
  if (phone == NULL) {
    printf("  => Voldemort not found in the table!\n\n");
  }

  // ----- SUMMARY -----
  printf("--- SUMMARY ---\n");
  printf("  Hash table uses an ARRAY of LINKED LISTS.\n");
  printf("  Best case:  O(1) - name is alone in its bucket\n");
  printf("  Worst case: O(n) - all names hash to the same bucket\n");
  printf("  The better the hash function, the fewer collisions!\n\n");

  // Free all allocated memory
  free_table();

  return 0;
}
