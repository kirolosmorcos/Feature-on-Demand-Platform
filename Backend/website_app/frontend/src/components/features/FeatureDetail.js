import React, { useEffect, useMemo, useState, useCallback } from "react";
import { useParams, useNavigate, Link } from "react-router-dom";
import featureService from "../../services/featureService";
import Loading from "../common/Loading";


function FeatureDetail({ showAlert }) {

    /* =========================
        STATE
    ========================== */

    const { featureCode } = useParams();
    const navigate = useNavigate();

    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);
    const [feature, setFeature] = useState(null);
    const [confirmDeleteCode, setConfirmDeleteCode] = useState(null);

    const featchFeature = useCallback(async () => {
        try {
            setLoading(true);
            setError(null);

            const data = await featureService.getFeatureByCode(featureCode);
            setFeature(data);
        } catch (err) {
            console.error("Failed to fetch feature:", err);
            setError("Unable to load feature. Please try again.");
            showAlert?.("error", "Failed to fetch feature");
        } finally {
            setLoading(false);
        }   

    }, [featureCode]);

    useEffect(() => {
        featchFeature();
    }, [featchFeature]);


    const handleDelete = async () => {
        if (confirmDeleteCode !== featureCode) {
            setConfirmDeleteCode(featureCode);
            return;
        }

        try {
            await featureService.deleteFeature(featureCode);
            showAlert?.("success", "Feature deleted successfully");
            navigate("/features");
        } catch (err) {
             console.error("Failed to delete feature:", err);
            showAlert?.("error", `Failed to delete feature ${featureCode}`);
        }   finally {
            setConfirmDeleteCode(null);
        }
    };

    if (loading) {
        return <Loading message="Loading features..." />;
    }

    if (error) {
        return <div className="alert alert-danger">{error}</div>;
    }

    if (!feature) return null;

    return (
        <div className="feature-detail">

        {/* HEADER */}
        <div className="d-flex justify-content-between align-items-center pt-3 pb-2 mb-3 border-bottom">
            <h1 className="h2">Feature: {feature.feature_name}</h1>

            <div>
            <Link
                to={`/features/edit/${feature.feature_code}`}
                className="btn btn-sm btn-outline-primary me-2"
            >
                <i className="bi bi-pencil me-1"></i> Edit
            </Link>

            <button
                           className={`btn btn-sm ${
                             confirmDeleteCode === feature.feature_code
                               ? "btn-danger"
                               : "btn-outline-danger"
                           }`}
                           onClick={() =>
                             handleDelete(feature.feature_code)
                           }
                         >
                           {confirmDeleteCode === feature.feature_code
                             ? "Confirm"
                             : <i className="bi bi-trash"></i>}
            </button>
            </div>
        </div>

        {/* BACK BUTTON */}
        

        <div className="row">

            {/* BASIC INFO */}
            <div className="col-md-6 mb-4">
            <div className="card">
                <div className="card-header">
                <strong>Basic Information</strong>
                </div>
                <div className="card-body">
                <p><strong>Name:</strong> {feature.feature_name}</p>
                <p><strong>Code:</strong> {feature.feature_code}</p>
                </div>
            </div>
            </div>

            {/* PRODUCTS */}
            <div className="col-md-6 mb-4">
            <div className="card">
                <div className="card-header">
                <strong>Products</strong>
                </div>
                <div className="card-body">
                {feature.products.length === 0 ? (
                    <div className="text-muted">No products available.</div>
                ) : (
                    <ul className="list-group">
                    {feature.products.map((p) => (
                        <li key={p.product_code} className="list-group-item">
                        {p.product_code} - ${p.price} ({p.duration})
                        </li>
                    ))}
                    </ul>
                )}
                </div>
            </div>
            </div>

        </div>

        {/* ECU REQUIREMENTS */}
        <div className="card">
            <div className="card-header">
            <strong>ECU Requirements</strong>
            </div>
            <div className="card-body">
            {feature.ecu_versions.length === 0 ? (
                <div className="text-muted">No ECU requirements defined.</div>
            ) : (
                <ul className="list-group">
                {feature.ecu_versions.map((r, index) => (
                    <li key={index} className="list-group-item">
                    {r.ecu} ({r.ecuModel}) - Version: {r.version}
                    </li>
                ))}
                </ul>
            )}
            </div>
        </div>

        <div className="d-flex justify-content-start mt-5 mb-4">
            <button
            className="btn btn-outline-secondary"
            onClick={() => navigate("/features")}
            >
            ← Back to Features
            </button>
        </div>

        </div>
    );
    }

    export default FeatureDetail;



        

        

