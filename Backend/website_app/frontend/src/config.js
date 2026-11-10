const API_CONFIG = {
  // baseURL: process.env.REACT_APP_API_URL || "http://localhost:5000/api/",
  baseURL: process.env.REACT_APP_API_URL || "/api/",
  endpoints: {
    carTypes: "car-types",
    ecus: "ecus",
    versions: "versions",
    features: "features",
    requests: "requests",
  },
};

export default API_CONFIG;
