const path = require('path');
const CopyPlugin = require("copy-webpack-plugin");

module.exports = {
  entry: './browser/index.js',
  output: {
    filename: 'test.build.js',
    path: path.resolve(__dirname, 'dist'),
    publicPath: '/browser'
  },
  devServer: {
    open: true,
    openPage: '/browser'
  },
  mode: "development",
  plugins: [
    new CopyPlugin({
      patterns: [
        { from: "../../../build/source/JsMaterialX/JsMaterialX.wasm" },
        { from: "../../../build/source/JsMaterialX/JsMaterialX.js" },
        { from: "../../../build/source/JsMaterialX/JsMaterialX.data" },
      ],
    }),
  ],
  externals: {
    JsMaterialX: 'JsMaterialX',
  }
};
