"""
Copyright (c) 2019 Cypress Semiconductor Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

"""
Script to generate JWT certificate for Cypress Bootloader
"""
import os
import json
import struct
import hashlib
from datetime import datetime
import click
from jwcrypto import jwk, jwt
from intelhex import IntelHex

IMAGE_ID = 0
IMAGE_TOC3_ADDR = int('0x100DFE00', 0)
CERT_EXP = datetime(2019, 12, 31)
IMAGE_ADDR_OFFST = 16
IMAGE_SIZE_OFFST = 20
IMAGE_VERSION_OFFST = 24
IMAGE_BUILD_OFFS = 28

def calculate_image_hash(image, toc3_addr):
    """
    Calculate image hash and read some metadata from the image
    """
    ih = IntelHex(image)
    ih.padding = 0x00
    segments = ih.segments()
    toc3_size_arr = ih.tobinarray(start=segments[1][0], size=4)
    toc3_size = struct.unpack('<I', toc3_size_arr)[0]

    image_addr_arr = ih.tobinarray(start=(segments[1][0] + IMAGE_ADDR_OFFST), size=4)
    image_addr = struct.unpack('<I', image_addr_arr)[0]

    image_size_arr = ih.tobinarray(start=(segments[1][0] + IMAGE_SIZE_OFFST), size=4)
    image_size = struct.unpack('<I', image_size_arr)[0]

    image_version = ih.tobinarray(start=(segments[1][0] + IMAGE_VERSION_OFFST), size=3)
    image_build = ih.tobinarray(start=(segments[1][0] + IMAGE_BUILD_OFFS), size=2)

    sha256 = hashlib.sha256()
    sha256.update(ih.tobinarray(start=image_addr, size=image_size))
    sha256.update(ih.tobinarray(start=toc3_addr, size=toc3_size))

    image_hash = sha256.digest()

    img_ver_list = [str(i) for i in reversed(image_version.tolist())]
    img_ver_list.append(str(struct.unpack('<H', image_build)[0]))

    image_attr = {}
    image_attr['image_hash'] = image_hash
    image_attr['image_addr'] = image_addr
    image_attr['image_version'] = '.'.join(img_ver_list)

    return image_attr

def load_key(key):
    """
    Load JWK for certificate signing
    """
    key_str = key.read()
    key_json = json.loads(key_str)
    return key_json

def create_cert_payload(image, image_attr, key, img_id, exp):
    """
    Create payload for the image certificate
    """
    # Remove private part from JWK
    key_pub = key['cy_pub_key']

    payload = {}
    payload["image_id"] = img_id
    payload["image_hash"] = list(image_attr['image_hash'])
    payload["image_file"] = os.path.basename(image)
    payload["image_address"] = image_attr['image_addr']
    payload["image_version"] = image_attr['image_version']
    payload["policy_template"] = ""
    payload["cy_pub_key"] = key_pub
    payload["iat"] = int(datetime.now().timestamp())
    payload["exp"] = int(exp.timestamp())

    return payload

def create_certificate(payload, key, output):
    """
    Create JWT certificate
    """
    key_priv = key['cy_priv_key']
    token = jwt.JWT(header={"alg": "ES256"}, claims=payload)
    token.make_signed_token(jwk.JWK(**key_priv))
    token_str = token.serialize(compact=True)
    output.write(token_str)

@click.command()
@click.option('-i', '--image', 'image',
              type=click.Path(),
              default=None,
              help='Image in the Intel HEX format.')
@click.option('-k', '--key', 'key',
              type=click.File('r'),
              default=None,
              help='Private key in the JWK format to sign certificate.')
@click.option('-o', '--cert', 'img_cert',
              type=click.File('w'),
              default='out.jwt',
              help='Certificate for input image in the JWT format.')
def main(image, key, img_cert):
    """
    Entry point of the script
    """
    image_attr = calculate_image_hash(image, IMAGE_TOC3_ADDR)
    key_json = load_key(key)
    payload = create_cert_payload(image, image_attr, key_json, IMAGE_ID, CERT_EXP)
    create_certificate(payload, key_json, img_cert)
    print('Image certificate was created successfully.')

if __name__ == "__main__":
    main(None, None, None)
