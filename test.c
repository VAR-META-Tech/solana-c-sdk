#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "header/anchor_counter_interface.c"

const char *file_path = "wallet_keypair.json";
const char *file_path_payer = "wallet_keypair.json";
const char *file_path_recipient = "wallet_keypair_recipient.json";
const char *file_path_recipient2 = "wallet_keypair_recipient2.json";
const char *file_path_mint = "wallet_keypair_mint.json";
const char *devnet_url = "https://api.devnet.solana.com";

SolKeyPair *test_create_and_save_wallet(const char *file_path)
{
    printf("=== Test: Create and Save Wallet ===\n");
    SolKeyPair *wallet = create_and_save_wallet(file_path);

    // Check if the wallet loading succeeded
    if (wallet != NULL)
    {
        SolPublicKey *pub = get_public_key(wallet);
        printf("Wallet created and saved successfully.\n");

        // Print the loaded public key
        printf("Loaded Solana Wallet Public Key: %s\n", pub->data);
        // Print the wallet address
        printf("Loaded Solana Wallet Address: %s\n", get_wallet_address(wallet));
    }
    else
    {
        printf("Failed to load wallet.\n");
    }
    printf("=== End Test: Create and Save Wallet ===\n");
    return wallet;
}

SolKeyPair *test_create_and_save_mint_wallet()
{
    printf("Create mint wallet");
    return test_create_and_save_wallet(file_path_mint);
}

SolKeyPair *test_create_and_save_recipient_wallet()
{
    printf("Create Recipient wallet");
    return test_create_and_save_wallet(file_path_recipient);
}

SolKeyPair *test_create_and_save_recipient2_wallet()
{
    printf("Create Recipient2 wallet");
    return test_create_and_save_wallet(file_path_recipient2);
}

SolKeyPair *test_load_wallet_from_file(const char *file_path)
{
    SolKeyPair *wallet = load_wallet_from_file(file_path);
    // Check if the wallet loading succeeded
    if (wallet != NULL)
    {
        // SolPublicKey *pub = get_public_key(wallet);
        // // Print the loaded public key
        // printf("Loaded Solana Wallet Public Key: %s\n", pub->data);
        // // Print the wallet address
        // printf("Loaded Solana Wallet Address: %s\n", get_wallet_address(wallet));
    }
    else
    {
        printf("Failed to load wallet.\n");
    }
    return wallet;
}

SolClient *test_sol_client_new(const char *url)
{
    SolClient *client = new_sol_client(url);
    if (client != NULL)
    {
        printf("Solana Client created successfully.\n");
    }
    else
    {
        printf("Failed to create Solana Client.\n");
    }
    return client;
}

void test_sol_airdrop()
{
    SolClient *client = new_sol_client_with_commitment(devnet_url, 0);
    if (client != NULL)
    {
        SolKeyPair *wallet = load_wallet_from_file(file_path);
        if (wallet != NULL)
        {
            SolPublicKey *pub = get_public_key(wallet);
            uint64_t lamports = 100000000;
            bool success = request_airdrop(client, pub, lamports);
            if (success)
            {
                printf("Airdrop successful.\n");
            }
            else
            {
                printf("Airdrop failed.\n");
            }
            // get balance
            uint64_t balance = get_balance(client, pub);
            printf("Balance: %lu\n", balance);
        }
        else
        {
            printf("Failed to load wallet.\n");
        }
    }
    else
    {
        printf("Failed to create Solana Client.\n");
    }
}

void test_create_spl_token()
{
    SolClient *client = new_sol_client(devnet_url);
    if (client != NULL)
    {
        SolKeyPair *payer = load_wallet_from_file(file_path_payer);
        SolKeyPair *mint = load_wallet_from_file(file_path_mint);

        if (payer != NULL && mint != NULL)
        {
            printf("Solana mint Wallet Address: %s\n", get_wallet_address(mint));
            bool success = create_spl_token(client, payer, mint);
            if (success)
            {
                printf("SPL Token created successfully.\n");
            }
            else
            {
                printf("Failed to create SPL Token.\n");
            }

            SolMint *mint_info = get_mint_info(client, &mint->pubkey);
            if (mint_info != NULL)
            {
                printf("Mint Supply: %lu\n", mint_info->supply);
                printf("Mint Decimals: %u\n", mint_info->decimals);
                printf("Mint is initialized: %s\n", mint_info->is_initialized ? "true" : "false");
            }
            else
            {
                printf("Failed to get mint info.\n");
            }
        }
        else
        {
            printf("Failed to create wallets.\n");
        }
    }
    else
    {
        printf("Failed to create Solana Client.\n");
    }
}

void test_mint_spl_token()
{
    printf("=== Test: Mint SPL Token ===\n");

    // Create a Solana client
    SolClient *client = new_sol_client(devnet_url);
    if (client == NULL)
    {
        printf("Error: Failed to create Solana Client.\n");
        return;
    }

    // Load the payer wallet
    SolKeyPair *payer = load_wallet_from_file(file_path_payer);
    if (payer == NULL)
    {
        printf("Error: Failed to load payer wallet from file: %s\n", file_path_payer);
        return;
    }

    // Load the mint wallet
    SolKeyPair *mint = load_wallet_from_file(file_path_mint);
    if (mint == NULL)
    {
        printf("Error: Failed to load mint wallet from file: %s\n", file_path_mint);
        return;
    }

    // Load the recipient wallet
    SolKeyPair *recipient = load_wallet_from_file(file_path_payer);
    if (recipient == NULL)
    {
        printf("Error: Failed to load recipient wallet from file: %s\n", file_path_payer);
        return;
    }

    // Print recipient wallet address
    printf("Recipient Wallet Address: %s\n", get_wallet_address(recipient));

    // Define the amount to mint
    uint64_t amount = 1000000000000;

    // Perform the mint operation
    printf("Minting %lu tokens to recipient wallet...\n", amount);
    bool success = mint_spl(client, payer, mint, get_public_key(recipient), amount);
    if (success)
    {
        // Get the recipient's token balance
        uint64_t balance = get_associated_token_balance(client, &recipient->pubkey, &mint->pubkey);
        printf("Success: SPL Token minted successfully.\n");
        printf("Recipient Token Balance: %lu\n", balance);
    }
    else
    {
        printf("Error: Failed to mint SPL Token.\n");
    }

    printf("=== End Test: Mint SPL Token ===\n");
}

void test_transfer_spl_token()
{
    SolClient *client = new_sol_client(devnet_url);
    if (client != NULL)
    {
        SolKeyPair *sender = load_wallet_from_file(file_path_payer);
        SolKeyPair *mint = load_wallet_from_file(file_path_mint);
        SolKeyPair *recipient = load_wallet_from_file(file_path_recipient);

        if (sender != NULL && mint != NULL && recipient != NULL)
        {
            SolPublicKey *recipient_pubkey = get_public_key(recipient);
            uint64_t amount = 500000000; // Transfer 500 tokens
            printf("Solana Token Transfer to  Wallet Address: %s\n", get_wallet_address(recipient));
            bool success = transfer_spl(client, sender, recipient_pubkey, &mint->pubkey, amount);
            if (success)
            {
                printf("SPL Token transferred successfully.\n");
            }
            else
            {
                printf("Failed to transfer SPL Token.\n");
            }
        }
        else
        {
            printf("Failed to load wallets for transfer.\n");
        }
    }
    else
    {
        printf("Failed to create Solana Client.\n");
    }
}

void test_transfer_sol()
{
    SolClient *client = new_sol_client(devnet_url);
    if (client != NULL)
    {
        SolKeyPair *sender = load_wallet_from_file(file_path_payer);
        SolKeyPair *recipient_wallet = load_wallet_from_file(file_path_recipient);

        if (sender != NULL && recipient_wallet != NULL)
        {
            SolPublicKey *signer_pubkey = get_public_key(sender);
            SolPublicKey *recipient_pubkey = get_public_key(recipient_wallet);
            uint64_t lamports = 1000000; // Transfer 0.001 SOL

            printf("Transferring %lu lamports (%.9f SOL) to Wallet Address: %s\n", lamports, lamports / 1e9, get_wallet_address(recipient_wallet));

            bool success = transfer_sol(client, sender, recipient_pubkey, lamports);

            if (success)
            {
                printf("Successfully transferred %lu lamports (%.9f SOL).\n", lamports, lamports / 1e9);
            }
            else
            {
                printf("Failed to transfer SOL.\n");
            }

            // Check balances after transfer
            uint64_t signer_balance = get_balance(client, signer_pubkey);
            uint64_t recipient_balance = get_balance(client, recipient_pubkey);

            printf("Signer Balance: %lu lamports (%.9f SOL)\n", signer_balance, signer_balance / 1e9);
            printf("Recipient Balance: %lu lamports (%.9f SOL)\n", recipient_balance, recipient_balance / 1e9);
        }
        else
        {
            printf("Failed to load wallets for SOL transfer.\n");
        }
    }
    else
    {
        printf("Failed to create Solana Client.\n");
    }
}

void test_get_all_tokens()
{
    printf("=== Test: Get All Tokens ===\n");
    SolClient *client = new_sol_client(devnet_url);
    if (client == NULL)
    {
        printf("Failed to create Solana Client.\n");
        return;
    }

    SolKeyPair *wallet = test_load_wallet_from_file(file_path_payer);
    if (wallet == NULL)
    {
        printf("Failed to load wallet.\n");
        return;
    }

    SolPublicKey *wallet_pubkey = get_public_key(wallet);
    if (wallet_pubkey == NULL)
    {
        printf("Failed to get wallet public key.\n");
        return;
    }

    TokenList *tokens = get_all_tokens(client, wallet_pubkey);
    if (!tokens)
    {
        printf("Failed to fetch tokens.\n");
        return;
    }

    uintptr_t len = tokens->len;
    TokenInfo *data = tokens->data;
    printf("Total Tokens: %lu\n", len);
    for (uintptr_t i = 0; i < len; i++)
    {
        printf("Token Mint: %s, Balance: %s, Owner: %s\n",
               data[i].mint, data[i].balance, data[i].owner);
    }

    free_token_list(tokens);
    printf("=== End Test: Get All Tokens ===\n");
}

void test_counter()
{
    // Try run test anchor program https://github.com/thanhngoc541/anchor-counter, deploy the project to get program id
    // RPC URL and paths
    const char *rpc_url = "https://api.devnet.solana.com";
    const char *payer_path = file_path;
    const char *program_id = "3CkKwWzHTvwnAURu8TD4JijeuYZkaPkU14QRGeGLHbSw";

    // Initialize Solana client and wallet
    SolClient *client = new_sol_client(rpc_url);
    SolKeyPair *payer = load_wallet_from_file(payer_path);
    SolKeyPair *account = new_keypair();

    // Get system program ID
    SolPublicKey SYSTEM_PROGRAM_ID = get_system_program_id();

    // Prepare to initialize account
    SolPublicKey initialize_accounts[3] = {
        account->pubkey,
        payer->pubkey,
        SYSTEM_PROGRAM_ID};

    SolKeyPair *initialize_signers[2] = {payer, account};

    // Call initialize
    char *initialize_result = anchor_counter_initialize_c(
        client,
        program_id,
        initialize_accounts,
        3,
        initialize_signers,
        2);

    if (initialize_result != NULL)
    {
        printf("Initialize Result: %s\n", initialize_result);
        free(initialize_result);
    }
    else
    {
        printf("❌ Failed to initialize account.\n");
    }

    // Call increment method
    SolPublicKey increment_accounts[2] = {
        account->pubkey,
        payer->pubkey};

    SolKeyPair *increment_signers[1] = {payer};

    for (int i = 0; i < 2; i++)
    {
        char *increment_result = anchor_counter_increment_c(
            client,
            program_id,
            increment_accounts,
            2,
            increment_signers,
            1);

        if (increment_result != NULL)
        {
            printf("Increment Result: %s\n", increment_result);
            free(increment_result);
        }
        else
        {
            printf("❌ Failed to increment account.\n");
        }

        // Fetch and print the updated account value
        uint8_t account_data[512]; // Larger buffer to handle future expansion
        size_t data_offset = 8;
        size_t bytes_copied = get_account_data_c(
            client,
            &account->pubkey,
            account_data,
            sizeof(account_data),
            data_offset);

        if (bytes_copied > 0)
        {
            printf("✅ Data Copied: %zu bytes\n", bytes_copied);
            uint64_t *counter = (uint64_t *)account_data;
            printf("🔢 Counter Value: %lu\n", *counter);
        }
        else
        {
            printf("❌ Failed to fetch account data.\n");
        }
    }

    // Clean up resources
    free_client(client);
    free_payer(payer);
}

void measure_time(const char *test_name, void (*func)())
{
    struct timeval start, end;
    gettimeofday(&start, NULL); // Start timing

    func(); // Run the function

    gettimeofday(&end, NULL); // End timing

    // Compute execution time in milliseconds
    double elapsed_time = ((end.tv_sec - start.tv_sec) * 1000.0) +
                          ((end.tv_usec - start.tv_usec) / 1000.0);

    printf("| %-30s | %-10.3f ms |\n", test_name, elapsed_time);
}

void test_wallet_creation() { create_wallet(); }
void test_wallet_loading() { test_load_wallet_from_file(file_path); }
void test_airdrop() { test_sol_airdrop(); }
void test_mint_token() { test_mint_spl_token(); }
void test_transfer_spl() { test_transfer_spl_token(); }
void test_transfer() { test_transfer_sol(); }
void test_smart_contract() { test_counter(); }
void test()
{
    // these measure_time are already included set up step time
    // if you want to measure only the function time, you can remove the set up time mannually

    printf("\n| **Function**                      | **Execution Time** |\n");
    printf("|-----------------------------------|------------------|\n");

    // measure_time("Wallet Creation", test_wallet_creation);
    // measure_time("Wallet Loading", test_wallet_loading);
    // measure_time("Airdrop Request", test_airdrop);
    // measure_time("Mint SPL Token", test_mint_token);
    // measure_time("Transfer SOL", test_transfer);
    // measure_time("Transfer SPL Token", test_transfer_spl);
    // measure_time("Test Smart Contract", test_smart_contract);
}

int main()
{

    SolClient *client = new_sol_client_with_commitment(devnet_url, 0);
    if (client != NULL)
    {
        SolKeyPair *wallet = load_wallet_from_file(file_path);
        if (wallet != NULL)
        {
            SolPublicKey *pub = get_public_key(wallet);
            uint64_t lamports = 100000000;

            struct timeval start, end;
            gettimeofday(&start, NULL); // Start timing

            bool success = request_airdrop_async(client, pub, lamports);

            gettimeofday(&end, NULL); // End timing

            // Compute execution time in milliseconds
            double elapsed_time = ((end.tv_sec - start.tv_sec) * 1000.0) +
                                  ((end.tv_usec - start.tv_usec) / 1000.0);

            printf("| %-30s | %-10.3f ms |\n", request_airdrop, elapsed_time);
            if (success)
            {
                printf("Airdrop successful.\n");
            }
            else
            {
                printf("Airdrop failed.\n");
            }
            // get balance
            uint64_t balance = get_balance(client, pub);
            printf("Balance: %lu\n", balance);
        }
        else
        {
            printf("Failed to load wallet.\n");
        }
    }
    else
    {
        printf("Failed to create Solana Client.\n");
    }
    return 0;
}
