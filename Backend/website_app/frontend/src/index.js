import React from "react";
import ReactDOM from "react-dom/client";
import App from "./App";

const resizeObserverMessages = [
  "ResizeObserver loop completed with undelivered notifications.",
  "ResizeObserver loop limit exceeded",
];

window.addEventListener("error", (event) => {
  if (resizeObserverMessages.includes(event.message)) {
    event.preventDefault();
    event.stopImmediatePropagation();
    
    // Explicitly hide the Webpack dev server overlays
    const webpackOverlay = document.getElementById("webpack-dev-server-client-overlay");
    const webpackOverlayDiv = document.getElementById("webpack-dev-server-client-overlay-div");
    
    if (webpackOverlay) webpackOverlay.style.display = "none";
    if (webpackOverlayDiv) webpackOverlayDiv.style.display = "none";
  }
});

// We can safely remove the window.onerror block, as the event listener above handles it!

const root = ReactDOM.createRoot(document.getElementById("root"));
root.render(<App />);