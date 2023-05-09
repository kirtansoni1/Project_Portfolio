#Python libraries
import rsa

#Gets the key from file and loads it into the variables
def load_keys():
    with open('pubkey.pem', 'rb') as f:
        pubKey = rsa.PublicKey.load_pkcs1(f.read())

    with open('privkey.pem', 'rb') as f:
        privKey = rsa.PrivateKey.load_pkcs1(f.read())

    return pubKey, privKey

#Returns the encrypted data
def encrypt(msg, key):
    return rsa.encrypt(msg.encode('ascii'), key)

#Returns the decrypted data
def decrypt(ciphertext, key):
    try:
        return rsa.decrypt(ciphertext, key).decode('ascii')
    except:
        return '' #Returns empty value if the ciphertext is not valid