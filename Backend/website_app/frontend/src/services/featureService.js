import api from "./api";
import API_CONFIG from "../config";

const baseEndpoint = API_CONFIG.endpoints.features;

const featureService = {

    /**
   * Create a new feature
   * @param {Object} featureData - Feature data
   * @returns {Promise<Object>} Result of operation
   */
  async createFeature(featureData) {
    return api.post(baseEndpoint, featureData);
  },
  
    /**
   * getAllFeatures - Fetches all features from the backend
   * @returns {Promise<Array>} List of features
   */
  async  getAllFeatures() {
    return api.get(baseEndpoint)|| [];
  },

    /**
   * deleteFeature - Deletes a feature by its code
   * @param {string} featureCode - Code of the feature to delete
   * @returns {Promise<Object>} Result of delete operation
   */
  async  deleteFeature(featureCode) {
    return api.delete(`${baseEndpoint}/${featureCode}`);
  },

    /**
   * getFeatureByCode - Fetches a single feature by its code
   * @param {string} featureCode - Code of the feature to fetch
   * @returns {Promise<Object>} Feature data
   */
  async  getFeatureByCode(featureCode) {
    return api.get(`${baseEndpoint}/${featureCode}`);
  },

  /**
   * updateFeature - Updates an existing feature
   * @param {string} featureCode - Code of the feature to update
   * @param {Object} featureData - Updated feature data
   * @returns {Promise<Object>} Result of update operation
   */
  async updateFeature(featureCode, featureData) {
    return api.put(`${baseEndpoint}/${featureCode}`, featureData);
  }
};

export default featureService;



