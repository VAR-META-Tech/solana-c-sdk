use solana_client::{rpc_client::RpcClient, rpc_config::RpcRequestAirdropConfig};
use solana_sdk::{commitment_config::CommitmentConfig, pubkey::Pubkey};
use std::ffi::{c_char, CStr};

use crate::wallet::SolPublicKey;

pub struct SolClient {
    pub rpc_client: RpcClient,
}

#[no_mangle]
pub extern "C" fn new_sol_client_with_commitment(
    url: *const c_char,
    commitment_level: u8,
) -> *mut SolClient {
    // Convert the C string to a Rust string
    let c_str = unsafe { CStr::from_ptr(url) };
    let url_str = match c_str.to_str() {
        Ok(str) => str,
        Err(_) => return std::ptr::null_mut(),
    };

    // Map `commitment_level` (u8) to a Solana Commitment Level
    let commitment_config = match commitment_level {
        0 => CommitmentConfig::processed(), // Fastest (low confirmation)
        1 => CommitmentConfig::confirmed(), // Default (medium confirmation)
        2 => CommitmentConfig::finalized(), // Safest (fully confirmed)
        _ => CommitmentConfig::confirmed(), // Default if invalid input
    };

    // Create a new Solana client with the specified commitment level
    let rpc_client = RpcClient::new_with_commitment(url_str.to_string(), commitment_config);
    let client = SolClient { rpc_client };

    println!(
        "✅ Created new SolClient with commitment level: {:?}",
        commitment_config.commitment
    );

    Box::into_raw(Box::new(client))
}

#[no_mangle]
pub extern "C" fn new_sol_client(url: *const c_char) -> *mut SolClient {
    // Convert the C string to a Rust string
    let c_str = unsafe { CStr::from_ptr(url) };
    let url_str = match c_str.to_str() {
        Ok(str) => str,
        Err(_) => return std::ptr::null_mut(),
    };

    // Create a new Solana client
    let rpc_client = RpcClient::new(url_str.to_string());
    let client = SolClient { rpc_client };
    Box::into_raw(Box::new(client))
}

#[no_mangle]
pub extern "C" fn get_balance(client: *mut SolClient, pubkey: *mut SolPublicKey) -> u64 {
    let client = unsafe {
        assert!(!client.is_null());
        &mut *client
    };

    let pubkey = unsafe {
        assert!(!pubkey.is_null());
        &*pubkey
    };

    let pubkey = Pubkey::new_from_array(pubkey.data);
    client.rpc_client.get_balance(&pubkey).unwrap_or(0)
}

#[no_mangle]
pub extern "C" fn request_airdrop(
    client: *mut SolClient,
    pubkey: *mut SolPublicKey,
    lamports: u64,
) -> bool {
    let client = unsafe {
        assert!(!client.is_null());
        &mut *client
    };

    let pubkey = unsafe {
        assert!(!pubkey.is_null());
        &*pubkey
    };

    let pubkey = Pubkey::new_from_array(pubkey.data);
    println!(
        "Requesting airdrop of {} lamports to pubkey: {:?}",
        lamports, pubkey
    );

    match client.rpc_client.request_airdrop(&pubkey, lamports) {
        Ok(signature) => {
            println!("Airdrop requested successfully. Signature: {:?}", signature);
            true
        }
        Err(err) => {
            println!("Failed to request airdrop: {:?}", err);
            false
        }
    }
}

#[no_mangle]
pub extern "C" fn request_airdrop_async(
    client: *mut SolClient,
    pubkey: *mut SolPublicKey,
    lamports: u64,
) -> bool {
    let client = unsafe {
        assert!(!client.is_null());
        &mut *client
    };

    let pubkey = unsafe {
        assert!(!pubkey.is_null());
        &*pubkey
    };

    let pubkey = Pubkey::new_from_array(pubkey.data);
    println!(
        "🚀 Requesting airdrop (Async) of {} lamports to pubkey: {:?}",
        lamports, pubkey
    );

    // ✅ Create a separate Tokio runtime to run async tasks
    std::thread::spawn(move || {
        let runtime = tokio::runtime::Runtime::new().expect("Failed to create Tokio runtime");

        runtime.block_on(async move {
            let recent_blockhash = match client.rpc_client.get_latest_blockhash() {
                Ok(blockhash) => blockhash,
                Err(err) => {
                    eprintln!("Error fetching latest blockhash: {:?}", err);
                    return;
                }
            };

            let config = RpcRequestAirdropConfig {
                commitment: Some(CommitmentConfig::processed()),
                recent_blockhash: Some(recent_blockhash.to_string()),
            };

            match client
                .rpc_client
                .request_airdrop_with_config(&pubkey, lamports, config)
            {
                Ok(signature) => println!("✅ Airdrop requested: {:?}", signature),
                Err(err) => println!("❌ Airdrop failed: {:?}", err),
            }
        });
    });

    true // Return immediately without blocking
}
