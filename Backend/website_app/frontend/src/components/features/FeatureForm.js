import React, { useState, useEffect } from "react";
import { useNavigate, useParams } from "react-router-dom";
import ecuService from "../../services/ecuService";
import versionService from "../../services/versionService";
import featureService from "../../services/featureService";

function FeatureForm({ showAlert }) {

    const navigate = useNavigate();
    const { featureCode } = useParams();
    const isEditMode = Boolean(featureCode);


    const [submitting, setSubmitting] = useState(false);
    const [availableEcus, setAvailableEcus] = useState([]);
    const [availableVersions, setAvailableVersions] = useState([]);
    const [formData, setFormData] = useState({
        feature_name: "",
        feature_code: "",
        products: [],
        ecu_versions: [],
        });
    const [newProduct, setNewProduct] = useState({
        product_code: "",
        price: "",
        duration: "month",
        });
    const [newEcuVersion, setNewEcuVersion] = useState({
        ecu: "",
        ecuModel: "",
        version: "",
        });


    useEffect(() => {

      fetchEcus();

      if (isEditMode) {
        fetchFeature();
      }
    }, []);

    const fetchEcus = async () => {
        try {
            const response = await ecuService.getAllEcus();
            const data = response?.data || response;
            setAvailableEcus(data || []);
        } catch (error) {
            showAlert("error", "Failed to load ECUs");
        }
        };

    const fetchFeature = async () => {
        try {
            const response = await featureService.getFeatureByCode(featureCode);
            const data = response?.data || response;

            setFormData({
                feature_name: data.feature_name,
                feature_code: data.feature_code,
                products: data.products || [],
                ecu_versions: data.ecu_versions || [],
            });
        } catch (error) {
            showAlert("error", "Failed to load feature");
        }
    };

    const handleEcuChange = async (value) => {
       
        // ✅ Split name and model
        const [ecuName, ecuModel] = value.split("|");

        setNewEcuVersion({
            ecu: ecuName,
            ecuModel: ecuModel,
            version: "",
        });

        try {
            const response = await versionService.getVersionsForECU(
            ecuName,
            ecuModel
            );

            const data = response?.data || response;
            setAvailableVersions(data || []);

        } catch (error) {
            showAlert("error", "Failed to load versions");
        }
    };

    const handleAddEcuVersion = () => {
        if (!newEcuVersion.ecu || !newEcuVersion.version) {
        showAlert("error", "Please select both ECU and version");
        return;
        }
        if (formData.ecu_versions.some(ev => ev.ecu === newEcuVersion.ecu && ev.ecuModel === newEcuVersion.ecuModel)) {
        showAlert("error", `ECU ${newEcuVersion.ecu} (${newEcuVersion.ecuModel}) is already added`);
        return;
        }
        setFormData((prev) => ({
        ...prev,
        ecu_versions: [...prev.ecu_versions, newEcuVersion],
        }));
        setNewEcuVersion({ ecu: "", ecuModel: "", version: "" });
    };

    const handleAddProduct = () => {
        if (!newProduct.product_code || !newProduct.price) {
        showAlert("error", "Please fill in all product fields");
        return;
        }

        const durationExists = formData.products.some(
        (p) => p.duration === newProduct.duration
        );

        if (durationExists) {
        showAlert("error", `A product with ${newProduct.duration} duration already exists`);
        return;
        }

        const productCodeExists = formData.products.some(
        (p) => p.product_code.toLowerCase() === newProduct.product_code.toLowerCase().trim()
        );

        if (productCodeExists) {
        showAlert("error", `A product with code "${newProduct.product_code}" already exists`);
        return;
        }


        setFormData((prev) => ({
        ...prev,
        products: [...prev.products, newProduct],
        }));
        setNewProduct({ product_code: "", price: "", duration: "month" });
    };

    const handleRemoveProduct = (index) => {
        setFormData((prev) => {
        const updated = [...prev.products];
        updated.splice(index, 1);

        return {
            ...prev,
            products: updated,
        };
        });

        showAlert("info", "Product removed");
    };

    const handleProductChange = (index, field, value) => {
        const updated = [...formData.products];

        if (field === "duration") {
          if (updated.some((p, i) => i !== index && p.duration === value)) {
              showAlert("error", `A product with ${value} duration already exists`);
              return;
          }
        }

        if (field === "price") {
          value = Number(value);
        }

        updated[index][field] = value;
        setFormData({ ...formData, products: updated });
    };

      

    const handleRemoveEcuVersion = (index) => {
        setFormData((prev) => {
        const updated = [...prev.ecu_versions];
        updated.splice(index, 1);

        return {
            ...prev,
            ecu_versions: updated,
        };
        });

        showAlert("info", "ECU version removed");
    };


    const handleSubmit = async (e) => {
        e.preventDefault();

        if (!formData.feature_name || !formData.feature_code) {
        showAlert("warning", "Feature name and code required");
        return;
        }

        if (formData.products.length === 0) {
        showAlert("warning", "Add at least one product");
        return;
        }

        if (formData.ecu_versions.length === 0) {
        showAlert("warning", "Add at least one ECU & version");
        return;
        }

        try {
        setSubmitting(true);

        if (isEditMode) {
          await featureService.updateFeature(featureCode, formData);
          showAlert("success", "Feature updated successfully");
        } else {

          await featureService.createFeature(formData);

          showAlert("success", "Feature created successfully");
        }

        navigate("/features");

        } catch (error) {
          const message =
            error.response?.data?.error || "Failed to create feature";

          showAlert("error", message);
        } finally {
        setSubmitting(false);
        }
    };

    return (
      <div className="feature-form">
        <div className="d-flex justify-content-between align-items-center pt-3 pb-2 mb-3 border-bottom">
          <h1 className="h2">
            {isEditMode ? "Edit Feature" : "Add New Feature"}
          </h1>
        </div>

        <form onSubmit={handleSubmit}>
          <div className="row mb-4">

            {/* LEFT CARD */}
            <div className="col-md-4">
              <div className="card">
                <div className="card-header">
                  <h5>Feature Information</h5>
                </div>
                <div className="card-body">

                  <div className="mb-3">
                    <label className="form-label">Feature Name</label>
                    <input
                      type="text"
                      className="form-control"
                      value={formData.feature_name}
                      readOnly={isEditMode}
                      onChange={e =>
                        setFormData({ ...formData, feature_name: e.target.value })
                      }
                    />
                  </div>

                  <div className="mb-3">
                    <label className="form-label">Feature Code</label>
                    <input
                      type="text"
                      className="form-control"
                      value={formData.feature_code}
                      readOnly={isEditMode}
                      onChange={e =>
                        setFormData({ ...formData, feature_code: e.target.value })
                      }
                    />
                  </div>

                </div>
              </div>
            </div>

            {/* RIGHT SIDE */}
            <div className="col-md-8">

              {/* PRODUCTS */}
              <div className="card mb-4">
                <div className="card-header">
                  <h5>Products</h5>
                </div>
                <div className="card-body">

                  {/* Add Product */}
                  <div className="row g-3 mb-3">
                    <div className="col-md-4">
                      <input
                        type="text"
                        className="form-control"
                        placeholder="Product Code"
                        value={newProduct.product_code}
                        onChange={e =>
                          setNewProduct({
                            ...newProduct,
                            product_code: e.target.value,
                          })
                        }
                      />
                    </div>

                    <div className="col-md-3">
                      <input
                        type="number"
                        className="form-control"
                        placeholder="Price"
                        value={newProduct.price}
                        onChange={e =>
                          setNewProduct({
                            ...newProduct,
                            price: Number(e.target.value),
                          })
                        }
                      />
                    </div>

                    <div className="col-md-3">
                      <select
                        className="form-select"
                        value={newProduct.duration}
                        onChange={e =>
                          setNewProduct({
                            ...newProduct,
                            duration: e.target.value,
                          })
                        }
                      >
                        <option value="month">Month</option>
                        <option value="year">Year</option>
                        <option value="lifetime">Lifetime</option>
                      </select>
                    </div>

                    <div className="col-md-2">
                      <button
                        type="button"
                        className="btn btn-primary w-100"
                        onClick={handleAddProduct}
                      >
                        Add
                      </button>
                    </div>
                  </div>

                  {/* Existing Products */}
                  {formData.products.map((product, index) => (
                    <div
                      key={index}
                      className="list-group-item d-flex justify-content-between align-items-center mb-2"
                    >
                      <div className="d-flex gap-2 flex-wrap align-items-center">
                        <strong>{product.product_code}</strong>

                        <input
                          type="number"
                          className="form-control"
                          style={{ width: "120px" }}
                          value={product.price}
                          onChange={e =>
                            handleProductChange(index, "price", e.target.value)
                          }
                        />

                        <select
                          className="form-select"
                          style={{ width: "150px" }}
                          value={product.duration}
                          onChange={e =>
                            handleProductChange(index, "duration", e.target.value)
                          }
                        >
                          <option value="month">Month</option>
                          <option value="year">Year</option>
                          <option value="lifetime">Lifetime</option>
                        </select>
                      </div>

                      <button
                        type="button"
                        className="btn btn-sm btn-outline-danger"
                        onClick={() => handleRemoveProduct(index)}
                      >
                        <i className="bi bi-trash"></i>
                      </button>
                    </div>
                  ))}

                </div>
              </div>

              {/* ECU */}
              <div className="card">
                <div className="card-header">
                  <h5>ECU & Versions</h5>
                </div>
                <div className="card-body">

                  <div className="row g-3 mb-3">
                    <div className="col-md-5">
                      <select
                        className="form-select"
                        onChange={e => handleEcuChange(e.target.value)}
                      >
                        <option value="">Select ECU</option>
                        {availableEcus.map(ecu => (
                          <option
                            key={`${ecu.name}|${ecu.model_number}`}
                            value={`${ecu.name}|${ecu.model_number}`}
                          >
                            {ecu.name} - {ecu.model_number}
                          </option>
                        ))}
                      </select>
                    </div>

                    <div className="col-md-5">
                      <select
                        className="form-select"
                        value={newEcuVersion.version}
                        onChange={e =>
                          setNewEcuVersion({
                            ...newEcuVersion,
                            version: e.target.value,
                          })
                        }
                      >
                        <option value="">Select Version</option>
                        {availableVersions.map((v, i) => (
                          <option key={i} value={v.version_number}>
                            {v.version_number}
                          </option>
                        ))}
                      </select>
                    </div>

                    <div className="col-md-2">
                      <button
                        type="button"
                        className="btn btn-primary w-100"
                        onClick={handleAddEcuVersion}
                      >
                        Add
                      </button>
                    </div>
                  </div>

                  {formData.ecu_versions.map((ev, index) => (
                    <div
                      key={index}
                      className="list-group-item d-flex justify-content-between align-items-center mb-2"
                    >
                      {ev.ecu} ({ev.ecuModel}) - {ev.version}

                      <button
                        type="button"
                        className="btn btn-sm btn-outline-danger"
                        onClick={() => handleRemoveEcuVersion(index)}
                      >
                        <i className="bi bi-trash"></i>
                      </button>
                    </div>
                  ))}

                </div>
              </div>

            </div>
          </div>

          {/* FOOTER */}
          <div className="d-flex justify-content-between">
            <button
              type="button"
              className="btn btn-secondary"
              onClick={() => navigate("/features")}
            >
              Cancel
            </button>

            <button
              type="submit"
              className="btn btn-success"
              disabled={submitting}
            >
              {submitting
                ? "Saving..."
                : isEditMode
                ? "Update Feature"
                : "Add Feature"}
            </button>
          </div>

        </form>
      </div>
    );
  }

  export default FeatureForm;