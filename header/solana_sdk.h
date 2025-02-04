#ifdef __cplusplus
extern "C" {
#endif


#ifndef SOLANA_SDK_H
#define SOLANA_SDK_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct SolClient SolClient;

typedef struct SolPublicKey {
  uint8_t data[32];
} SolPublicKey;

typedef struct SolKeyPair {
  uint8_t bytes[64];
  struct SolPublicKey pubkey;
} SolKeyPair;

typedef struct TokenInfo {
  const char *mint;
  const char *balance;
  const char *owner;
} TokenInfo;

typedef struct TokenList {
  struct TokenInfo *data;
  uintptr_t len;
} TokenList;

typedef struct SolMint {
  struct SolPublicKey *mint_authority;
  uint64_t supply;
  uint8_t decimals;
  bool is_initialized;
  struct SolPublicKey *freeze_authority;
} SolMint;

typedef struct SolSecretKey {
  uint8_t data[64];
} SolSecretKey;

struct SolClient *new_sol_client_with_commitment(const char *url, uint8_t commitment_level);

struct SolClient *new_sol_client(const char *url);

uint64_t get_balance(struct SolClient *client, struct SolPublicKey *pubkey);

bool request_airdrop(struct SolClient *client, struct SolPublicKey *pubkey, uint64_t lamports);

bool request_airdrop_async(struct SolClient *client,
                           struct SolPublicKey *pubkey,
                           uint64_t lamports);

struct SolPublicKey get_system_program_id(void);

uintptr_t get_account_data_c(struct SolClient *client,
                             struct SolPublicKey *account_pubkey,
                             uint8_t *data_ptr,
                             uintptr_t data_len,
                             uintptr_t data_offset);

char *send_generic_transaction_c(struct SolClient *client,
                                 const char *program_id,
                                 const char *method_name,
                                 const struct SolPublicKey *account_pubkeys,
                                 uintptr_t account_count,
                                 struct SolKeyPair *const *signers,
                                 uintptr_t signer_count,
                                 const uint8_t *data_ptr,
                                 uintptr_t data_len);

void initialize_account_c(struct SolClient *client,
                          struct SolKeyPair *payer,
                          struct SolKeyPair *account,
                          const char *program_id);

void free_client(struct SolClient *client);

void free_payer(struct SolKeyPair *payer);

struct TokenList *get_all_tokens(struct SolClient *client, struct SolPublicKey *wallet);

struct TokenInfo *token_list_get_data(const struct TokenList *list);

uintptr_t token_list_get_len(const struct TokenList *list);

void free_token_list(struct TokenList *list);

bool transfer_sol(struct SolClient *client,
                  struct SolKeyPair *sender,
                  struct SolPublicKey *recipient,
                  uint64_t lamports);

bool transfer_spl(struct SolClient *client,
                  struct SolKeyPair *sender,
                  struct SolPublicKey *recipient,
                  struct SolPublicKey *mint,
                  uint64_t amount);

bool create_spl_token(struct SolClient *client, struct SolKeyPair *payer, struct SolKeyPair *mint);

struct SolMint *get_mint_info(struct SolClient *client, struct SolPublicKey *mint_pubkey);

struct SolPublicKey *get_or_create_associated_token_account(struct SolClient *client,
                                                            struct SolKeyPair *payer,
                                                            struct SolPublicKey *owner,
                                                            struct SolKeyPair *mint);

bool mint_spl(struct SolClient *client,
              struct SolKeyPair *payer,
              struct SolKeyPair *mint_authority,
              struct SolPublicKey *recipient,
              uint64_t amount);

uint64_t get_associated_token_balance(struct SolClient *client,
                                      struct SolPublicKey *owner,
                                      struct SolPublicKey *mint);

struct SolPublicKey *get_public_key(struct SolKeyPair *wallet);

struct SolSecretKey *get_secret_key(struct SolKeyPair *wallet);

char *get_wallet_address(struct SolKeyPair *wallet);

struct SolKeyPair *create_wallet(void);

struct SolKeyPair *create_and_save_wallet(const char *file_path);

struct SolKeyPair *new_keypair(void);

struct SolKeyPair *load_wallet_from_file(const char *file_path);

struct SolPublicKey *get_pubkey_from_address(const char *address);

char *get_address_from_pubkey(const struct SolPublicKey *pubkey);

#endif  /* SOLANA_SDK_H */

#ifdef __cplusplus
}
#endif
