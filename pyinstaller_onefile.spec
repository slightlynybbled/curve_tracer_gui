# -*- mode: python -*-

block_cipher = None

image_files = [('curve_tracer/images/*.png', 'curve_tracer/images'), ('curve_tracer/images/*.ico', 'curve_tracer/images')]

a = Analysis(['curve_tracer\\__main__.py'],
             pathex=['C:\\_code\\curve_tracer_gui'],
             binaries=[],
             datas=image_files,
             hiddenimports=[],
             hookspath=[],
             runtime_hooks=[],
             excludes=[],
             win_no_prefer_redirects=False,
             win_private_assemblies=False,
             cipher=block_cipher)

pyz = PYZ(
    a.pure,
    a.zipped_data,
    cipher=block_cipher
)

exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          name='curve_tracer',
          debug=False,
          strip=False,
          upx=True,
          console=False,
          icon='curve_tracer/images/forembed.ico')
