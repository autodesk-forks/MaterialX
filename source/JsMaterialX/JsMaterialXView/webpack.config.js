const path = require('path');
const CopyPlugin = require("copy-webpack-plugin");

module.exports = {
  entry: './src/index.js',
  output: {
    filename: 'main.js',
    path: path.resolve(__dirname, 'dist')
  },
  mode: "development",
  plugins: [
    new CopyPlugin({
      patterns: [
        { from: "../../../resources" },
        { from: "../../../build/bin/JsMaterialXGenShader.wasm" },
        { from: "../../../build/bin/JsMaterialXGenShader.js" },
        { from: "../../../build/bin/JsMaterialXGenShader.data" },
      ],
    }),
  ],
  externals: {
    JsMaterialX: 'JsMaterialX',
  },
};