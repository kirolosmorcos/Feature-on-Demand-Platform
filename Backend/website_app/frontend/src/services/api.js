import API_CONFIG from "../config";

// Generic API service with methods for common HTTP requests

function getToken() {
  return localStorage.getItem("token");
}

function logoutAndRedirect() {
  localStorage.removeItem("token");
  localStorage.removeItem("user");
  window.location.href = "/login";
}

function buildUrl(endpoint, params = {}) {
  const base = API_CONFIG.baseURL.replace(/\/?$/, "/");
  const ep = endpoint.replace(/^\/+/, "");
  const url = new URL(`${base}${ep}`, window.location.origin);

  Object.keys(params).forEach((key) => {
    if (params[key] !== undefined && params[key] !== null) {
      url.searchParams.append(key, params[key]);
    }
  });

  return url.toString();
}

function getJsonHeaders() {
  const token = getToken();

  return {
    "Content-Type": "application/json",
    Accept: "application/json",
    ...(token ? { Authorization: `Bearer ${token}` } : {}),
  };
}

function getDownloadHeaders() {
  const token = getToken();

  return {
    ...(token ? { Authorization: `Bearer ${token}` } : {}),
  };
}

async function handleJsonResponse(response) {
  const text = await response.text();

  let data = {};

  try {
    data = text ? JSON.parse(text) : {};
  } catch {
    throw new Error(text || "Backend returned a non-JSON response");
  }

  if (response.status === 401) {
    logoutAndRedirect();
    throw new Error(
      data.msg || data.message || data.error || "Session expired. Please log in again."
    );
  }

  if (!response.ok) {
    throw new Error(
      data.error ||
        data.message ||
        data.msg ||
        `API error: ${response.status} ${response.statusText}`
    );
  }

  return data;
}

async function handleBlobResponse(response) {
  if (response.status === 401) {
    logoutAndRedirect();
    throw new Error("Session expired. Please log in again.");
  }

  if (!response.ok) {
    throw new Error(`API error: ${response.status} ${response.statusText}`);
  }

  return response.blob();
}

const api = {
  async get(endpoint, params = {}) {
    const url = buildUrl(endpoint, params);

    console.log("🔍 GET", url, "params:", params);

    try {
      const response = await fetch(url, {
        method: "GET",
        headers: getJsonHeaders(),
      });

      return await handleJsonResponse(response);
    } catch (error) {
      console.error("API GET request failed:", error);
      throw error;
    }
  },

  async post(endpoint, data = {}) {
    const url = buildUrl(endpoint);

    try {
      const response = await fetch(url, {
        method: "POST",
        headers: getJsonHeaders(),
        body: JSON.stringify(data),
      });

      return await handleJsonResponse(response);
    } catch (error) {
      console.error("API POST request failed:", error);
      throw error;
    }
  },

  async put(endpoint, data = {}) {
    const url = buildUrl(endpoint);

    try {
      const response = await fetch(url, {
        method: "PUT",
        headers: getJsonHeaders(),
        body: JSON.stringify(data),
      });

      return await handleJsonResponse(response);
    } catch (error) {
      console.error("API PUT request failed:", error);
      throw error;
    }
  },

  async patch(endpoint, data = {}) {
    const url = buildUrl(endpoint);

    try {
      const response = await fetch(url, {
        method: "PATCH",
        headers: getJsonHeaders(),
        body: JSON.stringify(data),
      });

      return await handleJsonResponse(response);
    } catch (error) {
      console.error("API PATCH request failed:", error);
      throw error;
    }
  },

  async delete(endpoint) {
    const url = buildUrl(endpoint);

    try {
      const response = await fetch(url, {
        method: "DELETE",
        headers: getJsonHeaders(),
      });

      return await handleJsonResponse(response);
    } catch (error) {
      console.error("API DELETE request failed:", error);
      throw error;
    }
  },

  async downloadFile(endpoint) {
    const url = buildUrl(endpoint);

    try {
      const response = await fetch(url, {
        method: "GET",
        headers: getDownloadHeaders(),
      });

      return await handleBlobResponse(response);
    } catch (error) {
      console.error("File download failed:", error);
      throw error;
    }
  },
};

export default api;